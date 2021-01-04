#pragma once

#include "stdafx.h"
#include <vector>
#include "mdf4.h"


using namespace std;


#ifndef WIN32
typedef unsigned long DWORD;
typedef long long __int64;
#endif



/* Signed-Unsigned conversion tables */
static M_UINT32 signed_max[33] = {
	0x00000000L, /* Gibts nicht */
	0x00000001UL - 1UL,  0x00000002UL - 1UL,  0x00000004UL - 1UL,  0x00000008UL - 1UL,  //lint !e778 (Info -- Constant expression evaluates to 0 in operation '-')
	0x00000010UL - 1UL,  0x00000020UL - 1UL,  0x00000040UL - 1UL,  0x00000080UL - 1UL,
	0x00000100UL - 1UL,  0x00000200UL - 1UL,  0x00000400UL - 1UL,  0x00000800UL - 1UL,
	0x00001000UL - 1UL,  0x00002000UL - 1UL,  0x00004000UL - 1UL,  0x00008000UL - 1UL,
	0x00010000UL - 1UL,  0x00020000UL - 1UL,  0x00040000UL - 1UL,  0x00080000UL - 1UL,
	0x00100000UL - 1UL,  0x00200000UL - 1UL,  0x00400000UL - 1UL,  0x00800000UL - 1UL,
	0x01000000UL - 1UL,  0x02000000UL - 1UL,  0x04000000UL - 1UL,  0x08000000UL - 1UL,
	0x10000000UL - 1UL,  0x20000000UL - 1UL,  0x40000000UL - 1UL,  0x80000000UL - 1UL
};

inline M_UINT32 get_signed_max(int idx)
{
	if (idx >= 0 && idx < 33)
		return signed_max[idx];
	return 0;
}

static M_UINT32 signed_sub[33] = {
	0x00000000UL, /* Gibts nicht */
	0x00000002UL,  0x00000004UL,  0x00000008UL,  0x00000010UL,
	0x00000020UL,  0x00000040UL,  0x00000080UL,  0x00000100UL,
	0x00000200UL,  0x00000400UL,  0x00000800UL,  0x00001000UL,
	0x00002000UL,  0x00004000UL,  0x00008000UL,  0x00010000UL,
	0x00020000UL,  0x00040000UL,  0x00080000UL,  0x00100000UL,
	0x00200000UL,  0x00400000UL,  0x00800000UL,  0x01000000UL,
	0x02000000UL,  0x04000000UL,  0x08000000UL,  0x10000000UL,
	0x20000000UL,  0x40000000UL,  0x80000000UL,  0x00000000UL
};

inline M_UINT32 get_signed_sub(int idx)
{
	if (idx >= 0 && idx < 33)
		return signed_sub[idx];
	return 0;
}
/*
static BYTE byte_mask[9] = {
0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,
0xff
};
*/
static M_UINT32 dword_mask[33] = {
	0x00000000L,0x00000001L,0x00000003L,0x00000007L,0x0000000fL,0x0000001fL,0x0000003fL,0x0000007fL,
	0x000000ffL,0x000001ffL,0x000003ffL,0x000007ffL,0x00000fffL,0x00001fffL,0x00003fffL,0x00007fffL,
	0x0000ffffL,0x0001ffffL,0x0003ffffL,0x0007ffffL,0x000fffffL,0x001fffffL,0x003fffffL,0x007fffffL,
	0x00ffffffL,0x01ffffffL,0x03ffffffL,0x07ffffffL,0x0fffffffL,0x1fffffffL,0x3fffffffL,0x7fffffffL,
	0xffffffffL
};

inline M_UINT32 get_dword_mask(int idx)
{
	if (idx >= 0 && idx < 33)
		return dword_mask[idx];
	return 0;
}
//lint -e799 (Info -- numerical constant '0x00ffffffffffffff' larger than unsigned long)

static M_UINT64 i64_mask[65] = {
	0x0000000000000000,0x0000000000000001,0x0000000000000003,0x0000000000000007,
	0x000000000000000f,0x000000000000001f,0x000000000000003f,0x000000000000007f,
	0x00000000000000ff,0x00000000000001ff,0x00000000000003ff,0x00000000000007ff,
	0x0000000000000fff,0x0000000000001fff,0x0000000000003fff,0x0000000000007fff,
	0x000000000000ffff,0x000000000001ffff,0x000000000003ffff,0x000000000007ffff,
	0x00000000000fffff,0x00000000001fffff,0x00000000003fffff,0x00000000007fffff,
	0x0000000000ffffff,0x0000000001ffffff,0x0000000003ffffff,0x0000000007ffffff,
	0x000000000fffffff,0x000000001fffffff,0x000000003fffffff,0x000000007fffffff,
	0x00000000ffffffff,0x00000001ffffffff,0x00000003ffffffff,0x00000007ffffffff,
	0x0000000fffffffff,0x0000001fffffffff,0x0000003fffffffff,0x0000007fffffffff,
	0x000000ffffffffff,0x000001ffffffffff,0x000003ffffffffff,0x000007ffffffffff,
	0x00000fffffffffff,0x00001fffffffffff,0x00003fffffffffff,0x00007fffffffffff,
	0x0000ffffffffffff,0x0001ffffffffffff,0x0003ffffffffffff,0x0007ffffffffffff,
	0x000fffffffffffff,0x001fffffffffffff,0x003fffffffffffff,0x007fffffffffffff,
	0x00ffffffffffffff,0x01ffffffffffffff,0x03ffffffffffffff,0x07ffffffffffffff,
	0x0fffffffffffffff,0x1fffffffffffffff,0x3fffffffffffffff,0x7fffffffffffffff,
	0xffffffffffffffff
};

inline M_UINT64 get_i64_mask(int idx)
{
	if (idx >= 0 && idx < 65)
		return i64_mask[idx];
	return 0;
}

inline void MovAndSwap(M_BYTE* pVal, M_BYTE* pDst, int nBytes)
{
	pDst += nBytes;
	while (nBytes--) *--pDst = *pVal++;
}

//////////////////////////////////////////////////////////////////////////


class _ReadMF4
{
public:
	_ReadMF4()
	{
		m_pDataStream = NULL;
		m_dwBufferSize = 0;
		m_iBufCtr = 0;
		m_nBuf = 0;
		m_iBufSiz = 0;
		m_pBuffer = NULL;
	}
	~_ReadMF4()
	{
		if (m_pDataStream)
			delete m_pDataStream;
	}

	BOOL Open(M4DGBlock *pdg, int iBufferSize, int iBufCnt)
	{
		m_iBufSiz = iBufferSize;
		m_dwBufferSize = iBufferSize * iBufCnt;
		m_pBuffer = calloc(1, m_dwBufferSize);
		if (!m_pBuffer)
			return FALSE;
		m_iBufCtr = 0;
		m_nBuf = iBufCnt;
		m_pDataStream = pdg->ReadStream();
		return m_pDataStream != NULL;
	}
	void Close(void)
	{
		if (m_pDataStream)
		{
			m_pDataStream->Close();
			delete m_pDataStream;
			m_pDataStream = NULL;
		}
		free(m_pBuffer);
		m_pBuffer = NULL;
	}
	int Seek(__int64 i64Pos)
	{
		m_iBufCtr = 0;
		if (m_pDataStream == NULL)
			return 0;
		return m_pDataStream->Seek(i64Pos) == i64Pos;
	}
	DWORD Read(DWORD dwSize, void *pBuffer)
	{
		BOOL bResult;
		DWORD dwRead = 0;
		if (m_iBufCtr++ == 0)
		{
			bResult = m_pDataStream->Read(m_dwBufferSize, m_pBuffer) == m_dwBufferSize;
			m_ptr = (char *)m_pBuffer;
		}
		else
		{
			bResult = TRUE;
			m_ptr += m_iBufSiz;
		}
		memcpy(pBuffer, m_ptr, dwSize);
		if (m_iBufCtr >= m_nBuf)
			m_iBufCtr = 0;
		if (bResult)
			return dwSize;
		return 0;
	}

	m4DataStream *m_pDataStream;
	void *m_pBuffer;
	char *m_ptr;
	DWORD m_dwBufferSize; // full buffer
	int   m_iBufSiz;      // single buffer size
	int   m_iBufCtr;
	int   m_nBuf;
};

struct HeaderBlock {

};


class CN;
class CG;

class DG {
	
	typedef vector<CG*>	ary_cgs;

public:
	DG(M4DGBlock* _dg, MDF4File* _m4) {
		m_block = _dg;
		m_m4 = _m4;
	}
	~DG() {
		clear();
	}

	M4DGBlock* get_block() const {
		return m_block;
	}

	MDF4File* get_mdf4_file() const {
		return m_m4;
	}

	ary_cgs& get_cgs() {
		return m_cgs;
	}

	void add_cg(CG* _cg) {
		if (_cg) {
			m_cgs.push_back(_cg);
		}
	}

protected:
private:
	M4DGBlock*	m_block;
	MDF4File*	m_m4;
	ary_cgs		m_cgs;

	void clear() {
		if (m_block) {
			delete m_block;
			m_block = nullptr;
		}
		for (auto &cg : m_cgs) {
			if (cg) {
				delete cg;
				cg = nullptr;
			}	
		}
		m_cgs.clear();
	}
};


class CG {
	
	typedef	vector<CN*>	ary_cns;
	
public:
	CG(M4CGBlock *_cg, DG* _dg) {
		m_block = _cg;
		m_dg = _dg;
	}
	~CG() {
		clear();
	}
	
	M4CGBlock* get_block() const {
		return m_block;
	}
	
	DG* get_dg() const {
		return m_dg;
	}
	
	ary_cns& get_cns(){
		return m_cns;
	}

	void add_cn(CN* _cn) {
		if (_cn) {
			m_cns.push_back(_cn);
		}
	}
	
protected:
private:
	M4CGBlock	*m_block;
	DG			*m_dg;
	ary_cns		m_cns;
	
	void clear() {
		if (m_block) {
			delete m_block;
			m_block = nullptr;
		}
		for (auto &cn : m_cns) {
			if (cn) {
				delete cn;
				cn = nullptr;
			}
		}
		m_cns.clear();
	}
};


class CN {
public:
	CN(M4CNBlock* _cn, CG* _cg) {
		m_block = _cn;
		m_cg = _cg;
	}
	~CN() {
		clear();
	}

	M4CNBlock* get_block() {
		return m_block;
	}

	CG* get_cg() const {
		return m_cg;
	}

	char* get_cn_tx(unsigned int linkNo) {
		if (m_cg) {
			char* ptx = nullptr;
			MDF4File* m4 = m_cg->get_dg()->get_mdf4_file();
			M4TXBlock* tx = (M4TXBlock*)m4->LoadLink(*m_block, linkNo);
			//if (tx) {
			//	ptx = get_string(tx);
			//	delete tx;
			//}
			ptx = get_string(tx);
			delete tx;
			return ptx;
		}
	}

	void get_value() {
		M4CGBlock* cgblk = m_cg->get_block();
		M4DGBlock* dgblk = m_cg->get_dg()->get_block();
		int iScanSize = cgblk->cg_data_bytes + cgblk->cg_inval_bytes + dgblk->dg_rec_id_size;
		M_UINT8* pScan = (M_UINT8*)calloc(1, iScanSize);
		int cnt = cgblk->cg_cycle_count;
		cnt = 20;

		for (int i = 0; i < cnt; i++) {
			BOOL bResult = get_record(dgblk, cgblk, pScan, i, i);
			if (!bResult)
				printf("        read error\n");
			else {
				double value;
				BOOL bNoval = get_raw_value(dgblk, cgblk, m_block, pScan, &value);
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

protected:
private:
	M4CNBlock*	m_block;
	CG*			m_cg;
	//unsigned long	m_idx;

	char* get_string(M4TXBlock* ptx) {
		char* pBuf = new char[4096];
		memset(pBuf, 0, 4096);
		if (ptx) {
			const unsigned char* t = ptx->m_utf8.data();
			//utf8in in(ptx->m_utf8.data());
			strncpy(pBuf, (char*)t, 4095);
		}
		return pBuf;
	}

	BOOL get_record(M4DGBlock* dg, M4CGBlock* cg, M_BYTE* ptr, M_UINT64 ix1, M_UINT64 ix2) {
		if (!dg || !cg)
			return FALSE;

		void* pOrgPtr = ptr;
		M_UINT32 RecLen = (M_UINT32)cg->getRecordSize() + dg->dg_rec_id_size;
		M_UINT64 dwPos = ix1 * RecLen;
		M_UINT64 n = ix2 - ix1 + 1;

		_ReadMF4 rmf4;
		if (!rmf4.Open(dg, RecLen, n)) {
			return FALSE;
		}

		if (!rmf4.Seek(dwPos)) {
			rmf4.Close();
			return FALSE;
		}

		DWORD dwSize = rmf4.Read(RecLen * n, ptr);
		rmf4.Close();

		return TRUE;
	}

	BOOL get_raw_value(M4DGBlock* dg, M4CGBlock* cg, M4CNBlock* cn, M_BYTE* pRecord, double* pValue)
	{
		BOOL bIsNoval = FALSE;
		M_BYTE* ibptr = NULL;
		M_BYTE* pInv = NULL;
		int iIBpos;
		M_UINT16 nbit, sbit;
		int nbytes;
		M_UINT64 buf;
		M_BYTE* ptr = (M_BYTE*)&buf;

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
		*pValue = get_value(pRecord, sbit, nbit, nbytes, bInteger, bSigned, bMotorola, bFloat);
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

	double get_value(M_BYTE* dptr, int sbit, int nbit, int nbytes, BOOL bInteger, BOOL bSigned, BOOL bMotorola, BOOL bFloat)
	{
		double val;
		M_UINT64 dummy = 0;
		void* ptr = (void*)&dummy;
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

	void clear() {
		if (m_block) {
			delete m_block;
			m_block = nullptr;
		}
	}
};