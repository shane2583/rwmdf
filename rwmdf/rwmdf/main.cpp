#include <QtCore/QCoreApplication>
#include "stdafx.h"
#include "_rwmdf.h"



//////////////////////////////////////////////////////////////////////////

const char *StrID(M_UINT16 hdrID)
{
	static char szBuffer[8];
	for (int i = 0; i < 4; i++)
	{
		szBuffer[i] = hdrID & 0xFF;
		hdrID >>= 8;
	}
	szBuffer[4] = 0;
	return szBuffer;
}

// Helpers
char* GetString(M4TXBlock *pTX)
{
	static char szBuffer[4096];
	memset(szBuffer, 0, sizeof(szBuffer));
	if (pTX)
	{
		const unsigned char *t = pTX->m_utf8.data();
		//utf8in in(pTX->m_utf8.data());
		strncpy(szBuffer, (char*)t, 4095);
	}
	return szBuffer;
}

m4Block* LoadLink(MDF4File &m4, m4Block *pParent, int linkNo)
{
	m4Block* pResult = m4.LoadBlock(pParent->getLink(linkNo));
	delete pParent;
	return pResult;
}

void DisplayGroup(MDF4File &m4, M4DGBlock* pDG, uint32_t cnt)
{
	if (!pDG) return;
	printf("\n\n");
	printf("  %d Channel Group = 0x%llX\n", cnt, pDG->getLink(M4DGBlock::dg_cg_first));
	printf("  %d Data Block = ", cnt);
	m4Block *b = NULL;
	M_LINK l = pDG->getLink(M4DGBlock::dg_data);
	if (l)
		b = m4.LoadLink(*pDG, M4DGBlock::dg_data);
	if (b)
		printf("0x%I64X ID = %s\n", l, StrID(b->hdrID()));
	else
		printf("0x%I64X\n", l);
	printf("  Record ID Size = %u\n", (unsigned int)pDG->dg_rec_id_size);
}

void DisplayChannelGroup(MDF4File &m4, M4CGBlock *cg, uint32_t cnt)
{
	char *p = " ";
	printf("  Channel Group:\n");
	printf("    %d Channel        = 0x%llX\n", cnt, cg->getLink(M4CGBlock::cg_cn_first));
	M4TXBlock *tx = (M4TXBlock *)m4.LoadLink(*cg, M4CGBlock::cg_tx_acq_name);
	if (tx) {
		p = GetString(tx);
		delete tx;
	}
	printf("    Acquisition Name   = %s\n", p);

	p = " ";
	M4SIBlock *si = (M4SIBlock *)m4.LoadLink(*cg, M4CGBlock::cg_si_acq_source);
	if (si) {
		tx = (M4TXBlock *)m4.LoadLink(*si, M4SIBlock::si_tx_name);
		if (tx) {
			p = GetString(tx);
			delete tx;
		}
		delete si;
	}
	printf("    Acquisition Source = %s\n", p);
	printf("    Record ID          = %llu\n", cg->cg_record_id);
	printf("    Cycle Count        = %llu\n", cg->cg_cycle_count);
	printf("    Data Bytes         = %lu\n", cg->cg_data_bytes);
	printf("    Invalid Bytes      = %lu\n", cg->cg_inval_bytes);
}

void DisplayChannel(MDF4File &m4, M4CNBlock *cn, int cnt)
{
	char *p = " ";
	printf("\n    Channel %d\n", cnt);
	M4TXBlock *tx = (M4TXBlock *)m4.LoadLink(*cn, M4CNBlock::cn_tx_name);
	if (tx) {
		p = GetString(tx);
		delete tx;
	}
	printf("      Name = %s\n", p);

	p = " ";
	M4SIBlock *si = (M4SIBlock *)m4.LoadLink(*cn, M4CNBlock::cn_si_source);
	if (si) {
		tx = (M4TXBlock *)m4.LoadLink(*si, M4SIBlock::si_tx_name);
		if (tx) {
			p = GetString(tx);
			delete tx;
		}
		delete si;
	}
	printf("      Source = %s\n", p);

	p = " ";
	tx = (M4TXBlock *)m4.LoadLink(*cn, M4CNBlock::cn_md_unit);
	if (tx) {
		p = GetString(tx);
		delete tx;
	}
	printf("      Unit = %s\n", p);

	printf("      Channel Type = %lu\n", (unsigned int)cn->cn_type);
	printf("      Data Type = %lu\n", (unsigned int)cn->cn_data_type);
	printf("      Bit Offset = %lu\n", (unsigned int)cn->cn_bit_offset);
	printf("      Byte Offset = %lu\n", (unsigned int)cn->cn_byte_offset);
	printf("      Bit Count = %lu\n", (unsigned int)cn->cn_bit_count);
	printf("      Flags = 0x%lX\n", (unsigned int)cn->cn_flags);
	printf("      Inval. Bit Pos = %lu\n", cn->cn_inval_bit_pos);
	printf("      Min Range Raw = %lf\n", cn->cn_val_range_min);
	printf("      Max Range Raw = %lf\n", cn->cn_val_range_max);
}

BOOL GetRecord(M4DGBlock *dg, M4CGBlock *cg, M_BYTE *ptr, M_UINT64 ix1, M_UINT64 ix2)
{
	if (!dg || !cg) 
		return FALSE;

	void *pOrgPtr = ptr;
	M_UINT32 RecLen = (M_UINT32)cg->getRecordSize() + dg->dg_rec_id_size;
	M_UINT64 dwPos = ix1 * RecLen;
	M_UINT64 n = ix2 - ix1 + 1;

	_ReadMF4 rmf4;
	if (!rmf4.Open(dg, RecLen, n)) {
		return FALSE;
	}


	if (!rmf4.Seek(dwPos)){
		rmf4.Close();
		return FALSE;
	}
	
	DWORD dwSize = rmf4.Read(RecLen * n, ptr);
	rmf4.Close();
	
	return TRUE;
}

double GetValueFromRecord(M_BYTE *dptr, int sbit, int nbit, int nbytes, BOOL bInteger, BOOL bSigned, BOOL bMotorola, BOOL bFloat)
{
	double val;
	M_UINT64 dummy = 0;
	void *ptr = (void*)&dummy;
	if (bFloat)
	{
		if (nbit == 32)
		{
			if (bMotorola)
				MovAndSwap((M_BYTE*)dptr, (M_BYTE*)ptr, nbytes);
			else
				*(long*)ptr = *((long*)dptr);
			val = (double)*((float*)ptr);
		}
		else
		{
			if (bMotorola)
				MovAndSwap((M_BYTE*)dptr, (M_BYTE*)ptr, nbytes);
			else
				*(double*)ptr = *((double*)dptr);
			val = *((double*)ptr);
		}
	}
	else if (bInteger && nbit > 32)
	{
		if (bMotorola)
			MovAndSwap((M_BYTE*)dptr, (M_BYTE*)ptr, nbytes);
		else
			memcpy(ptr, dptr, nbytes);
		M_UINT64 i64Val = *((M_UINT64*)ptr);
		i64Val = (i64Val >> sbit) & get_i64_mask(nbit);
		val = (double)i64Val;
	}
	else
	{
		if (bMotorola)
			MovAndSwap((M_BYTE*)dptr, (M_BYTE*)ptr, nbytes);
		else
			memcpy(ptr, dptr, nbytes);
		M_UINT32 dwValue = *(M_UINT32*)ptr;
		if (nbit > 16)
		{
			M_UINT32 get_dword_mask(int idx);
			M_UINT32 get_signed_max(int idx);
			dwValue = (dwValue >> sbit) & get_dword_mask(nbit);
			if (bSigned && dwValue > get_signed_max(nbit))
				val = (double)(long)dwValue;
			else
				val = (double)dwValue;
		}
		else
		{
			M_UINT16 wValue = (dwValue >> sbit) & (M_UINT16)get_dword_mask(nbit);
			if (bSigned && dwValue > get_signed_max(nbit))
			{
				if (nbit <= 8)
					val = (double)(char)dwValue;
				else
					val = (double)(short)dwValue;
			}
			else
				val = (double)dwValue;
		}
	}
	return val;
}

BOOL GetRawValueFromRecord(M4DGBlock *dg, M4CGBlock *cg, M4CNBlock *cn, M_BYTE *pRecord, double *pValue)
{
	BOOL bIsNoval = FALSE;
	M_BYTE *ibptr = NULL;
	M_BYTE *pInv = NULL;
	int iIBpos;
	M_UINT16 nbit, sbit;
	int nbytes;
	M_UINT64 buf;
	M_BYTE *ptr = (M_BYTE *)&buf;

	if (cn->cn_data_type >= CN_D_STRING)
		return TRUE;

	BOOL bInteger = cn->cn_data_type <= CN_D_SINT_BE;
	BOOL bSigned = cn->cn_data_type >= CN_D_SINT_LE;
	BOOL bMotorola = (cn->cn_data_type & 1);
	BOOL bFloat = cn->cn_data_type > CN_D_SINT_BE;

	ibptr = pRecord;
	sbit = cn->cn_bit_offset;
	pRecord += sbit >> 3; // Byte Offset
	pRecord += cn->cn_byte_offset;
	if (dg->dg_rec_id_size)
		pRecord += dg->dg_rec_id_size;
	sbit &= 7;
	nbit = cn->cn_bit_count;
	nbytes = (nbit + 7) >> 3;

	if (cn->cn_flags & CN_F_INVALID)
	{
		M_UINT32 RecLen = cg->getRecordSize() + dg->dg_rec_id_size;
		ibptr += RecLen - cg->cg_inval_bytes;
		iIBpos = cn->cn_inval_bit_pos;
		ibptr += iIBpos / 8;
		iIBpos = 1 << (iIBpos % 8);
		if (*ibptr & iIBpos)
			bIsNoval = TRUE;
	}
	*pValue = GetValueFromRecord(pRecord, sbit, nbit, nbytes, bInteger, bSigned, bMotorola, bFloat);
	if (!bIsNoval && bFloat)
	{
#ifdef WIN32
		if (!_finite(*pValue))
#else
		if (!finite(*pValue))
#endif
		{
			bIsNoval = TRUE;
			*pValue = 1E308;
		}
	}
	return bIsNoval;
}

void DisplayData(M4DGBlock *dg, M4CGBlock *cg, M4CNBlock *cn)
{
	int iScanSize = cg->cg_data_bytes + cg->cg_inval_bytes + dg->dg_rec_id_size;
	M_UINT8 *pScan = (M_UINT8*)calloc(1, iScanSize);
	int n = cg->cg_cycle_count;
	if (n > 10)
	{
		n = 10; // display only 10 values
		printf("      Data (only 10 values):\n");
	}
	else
		printf("      Data (%d values):\n", n);

	for (int i = 0; i < n; i++)
	{
		BOOL bResult = GetRecord(dg, cg, pScan, i, i);
		if (!bResult)
			printf("        read error\n");
		else
		{
			double value;
			BOOL bNoval = GetRawValueFromRecord(dg, cg, cn, pScan, &value);
			if (bNoval) 
				printf("        novalue %.15lg\n", value);
			else {
				printf("        %20.15lg \n", value);
				//CMdf4Calc * pCalc = new CMdf4Calc(cn, m_m4);
				//if (pCalc->m_pCC != NULL)
				//{
				//	value = pCalc->MdfCalc(value);
				//	printf("%20.15lg\n", value);
				//}
				//delete pCalc;
			}
		}
	}
}

void readMf4(char *filename) {

	MDF4File m_m4;
	if (!m_m4.Open(filename))
		return;

	printf("File %s\n", filename);

	// ID Block
	mdfFileId *pId = m_m4.GetIdBlk();
	printf("id_file  = %s\n", pId->id_file);
	printf("id_vers  = %s\n", pId->id_vers);
	printf("id_prog  = %s\n", pId->id_prog);
	printf("id_order = %s\n", pId->id_order == 0 ? "Intel" : "Motorola");
	printf("id_float = %s\n", pId->id_float == 0 ? "IEEE 754" : "(unsupported)");
	printf("id_ver   = %d\n", (int)pId->id_ver);

	// Show time: don't know how to handle local/GMT time under Linux
	M4HDBlock *pHdr = m_m4.GetHdr();
	printf("Time: %d\n", (long)(pHdr->hd_start_time.time_ns / 1000000000));

	uint32_t dg_cnt = 0;
	M4DGBlock *dg = (M4DGBlock*)m_m4.LoadLink(*pHdr, M4HDBlock::hd_dg_first);
	while (dg) {
		// show the block of data group.
		DisplayGroup(m_m4, dg, ++dg_cnt);

		uint32_t cg_cnt = 0;
		M4CGBlock *cg = (M4CGBlock*)m_m4.LoadLink(*dg, M4DGBlock::dg_cg_first);
		while (cg) {
			// show the block of channel group.
			DisplayChannelGroup(m_m4, cg, ++cg_cnt);

			uint32_t cn_cnt = 0;
			M4CNBlock *cn = (M4CNBlock *)m_m4.LoadLink(*cg, M4CGBlock::cg_cn_first, M4ID_CN);
			while (cn) {
				DisplayChannel(m_m4, cn, ++cn_cnt);

				DisplayData(dg, cg, cn);
				cn = (M4CNBlock *)m_m4.LoadLink(*cn, M4CNBlock::cn_cn_next, M4ID_CN);
			}

			cg = (M4CGBlock*)LoadLink(m_m4, cg, M4CGBlock::cg_cg_next);
		}

		dg = (M4DGBlock*)m_m4.LoadLink(*dg, M4DGBlock::dg_dg_next);
	}
}


int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	char *filename = "D:\\source\\rwmdf\\DataSpySampleDataFile.mf4";
    //const char *filename = "/home/shane/pro/asam_mf4_lib/DataSpySampleDataFile.mf4";

	_RWMDF mf4;
	mf4.open(filename);


	// ID Block
	mdfFileId* pId = mf4.get_fid();
	printf("id_file  = %s\n", pId->id_file);
	printf("id_vers  = %s\n", pId->id_vers);
	printf("id_prog  = %s\n", pId->id_prog);
	printf("id_order = %s\n", pId->id_order == 0 ? "Intel" : "Motorola");
	printf("id_float = %s\n", pId->id_float == 0 ? "IEEE 754" : "(unsupported)");
	printf("id_ver   = %d\n", (int)pId->id_ver);

	// Show time: don't know how to handle local/GMT time under Linux
	M4HDBlock* pHdr = mf4.get_hdr();
	printf("Time: %d\n", (long)(pHdr->hd_start_time.time_ns / 1000000000));

	unsigned long cnt =  mf4.get_dgs().size();
	printf("Channel Count: %d\n",cnt);
	printf("\n");

	for (DG* dg : mf4.get_dgs()) {
		MDF4File* m4file = dg->get_mdf4_file();

		for (auto &cg : dg->get_cgs()) {

			M4CGBlock *cgblk = cg->get_block();
			printf("    Record ID          = %llu\n", cgblk->cg_record_id);
			printf("    Cycle Count        = %llu\n", cgblk->cg_cycle_count);
			printf("    Data Bytes         = %lu\n", cgblk->cg_data_bytes);
			printf("    Invalid Bytes      = %lu\n", cgblk->cg_inval_bytes);
			printf("\n");

			for (CN* cn : cg->get_cns()) {

				M4CNBlock* cnblk = cn->get_block();
				char* name = cn->get_cn_tx(M4CNBlock::cn_tx_name);
				printf("      Channel Name = %s\n", name);
				delete name;
				char* unit = cn->get_cn_tx(M4CNBlock::cn_md_unit);
				printf("      Unit = %s\n", unit);
				delete unit;
				printf("      Channel Type = %lu\n", (unsigned int)cnblk->cn_type);
				printf("      Data Type = %lu\n", (unsigned int)cnblk->cn_data_type);
				printf("      Bit Offset = %lu\n", (unsigned int)cnblk->cn_bit_offset);
				printf("      Byte Offset = %lu\n", (unsigned int)cnblk->cn_byte_offset);
				printf("      Bit Count = %lu\n", (unsigned int)cnblk->cn_bit_count);
				printf("      Flags = 0x%lX\n", (unsigned int)cnblk->cn_flags);
				printf("      Inval. Bit Pos = %lu\n", cnblk->cn_inval_bit_pos);
				printf("      Min Range Raw = %lf\n", cnblk->cn_val_range_min);
				printf("      Max Range Raw = %lf\n", cnblk->cn_val_range_max);
				printf("\n");

				cn->get_value();

				printf("\n");
				
			}
		}
	}
	
	mf4.clear_dgs();

	//readMf4(filename);

	return a.exec();
}
