#include "_rwmdf.h"


_RWMDF::_RWMDF()
{
	m_pHdr = nullptr;
}

_RWMDF::~_RWMDF()
{
	clear_dgs();
}




bool _RWMDF::open(const char *filename) {
	if (!m_m4.Open(filename))
		return false;
	//printf("File %s\n", filename);

	// ID Block
	mdfFileId *pId = m_m4.GetIdBlk();
	printf("id_file  = %s\n", pId->id_file);
	printf("id_vers  = %s\n", pId->id_vers);
	printf("id_prog  = %s\n", pId->id_prog);
	printf("id_order = %s\n", pId->id_order == 0 ? "Intel" : "Motorola");
	printf("id_float = %s\n", pId->id_float == 0 ? "IEEE 754" : "(unsupported)");
	printf("id_ver   = %d\n", (int)pId->id_ver);

	// Show time: don't know how to handle local/GMT time under Linux
	m_pHdr = m_m4.GetHdr();
	printf("Time: %d\n", (long)(m_pHdr->hd_start_time.time_ns / 1000000000));

	load_dgs();

	return true;
}

M4DGBlock* _RWMDF::get_dg(unsigned long index) {
	unsigned long cnt = get_dg_count();

	if (0 > index || cnt <= index)
		return false;

	cnt = 0;
	M4DGBlock *dg = (M4DGBlock*)m_m4.LoadLink(*m_pHdr, M4HDBlock::hd_dg_first);
	while (dg) {
		if (cnt++ == index) {
			break;
		}
		dg = (M4DGBlock*)m_m4.LoadLink(*dg, M4DGBlock::dg_dg_next);
	}
	
	return dg;
}

unsigned long _RWMDF::get_dg_count() {
	if (!m_pHdr)
		return 0;
	unsigned long cnt = 0;
	M4DGBlock *dg = (M4DGBlock*)m_m4.LoadLink(*m_pHdr, M4HDBlock::hd_dg_first);
	while (dg) {
		cnt++;
		dg = (M4DGBlock*)m_m4.LoadLink(*dg, M4DGBlock::dg_dg_next);
	}
	return cnt;
}

//long _RWMDF::get_dgroups(ary_dgs & dgs) {
//	long ret = 0;
//
//	if (0 < dgs.size()) {
//		for (auto &dgblock : dgs) {
//			if (dgblock)
//				delete dgblock;
//		}
//		dgs.clear();
//	}
//
//	M4DGBlock *dg = (M4DGBlock*)m_m4.LoadLink(*m_pHdr, M4HDBlock::hd_dg_first);
//	while (dg) {
//		dgs.push_back(dg);
//		dg = (M4DGBlock*)m_m4.LoadLink(*dg, M4DGBlock::dg_dg_next);
//	}
//
//	return dgs.size();
//}

m4Block* _RWMDF::LoadLink(MDF4File &m4, m4Block *pParent, int linkNo)
{
	m4Block* pResult = m4.LoadBlock(pParent->getLink(linkNo));
	delete pParent;
	return pResult;
}

unsigned long _RWMDF::load_dgs() {

	M4DGBlock *dg_block = (M4DGBlock*)m_m4.LoadLink(*m_pHdr, M4HDBlock::hd_dg_first);
	while (dg_block) {
		DG* dg = new DG(dg_block);

		M4CGBlock *cg_block = (M4CGBlock*)m_m4.LoadLink(*dg_block, M4DGBlock::dg_cg_first);
		while (cg_block) {
			CG* cg = new CG(cg_block);

			M4CNBlock *cn_block = (M4CNBlock *)m_m4.LoadLink(*cg->get_block(), M4CGBlock::cg_cn_first, M4ID_CN);
			while (cn_block) {
				CN* cn = new CN(cn_block);
				cg->add_cn(cn);

				cn_block = (M4CNBlock *)m_m4.LoadLink(*cn_block, M4CNBlock::cn_cn_next, M4ID_CN);
			}

			dg->add_cg(cg);
			cg_block = (M4CGBlock*)LoadLink(m_m4, cg_block, M4CGBlock::cg_cg_next);
		}

		dg_block = (M4DGBlock*)m_m4.LoadLink(*dg_block, M4DGBlock::dg_dg_next);
		m_dgs.push_back(dg);
	}

	return m_dgs.size();
}

ary_dgs& _RWMDF::get_dgs() {
	return m_dgs;
}

void _RWMDF::clear_dgs() {
	for (auto &dg: m_dgs) {
		if (dg) {
			delete dg;
			dg = nullptr;
		}
	}
}


BOOL _RWMDF::get_record(M4DGBlock *dg, M4CGBlock *cg, M_BYTE *ptr, M_UINT64 ix1, M_UINT64 ix2)
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


	if (!rmf4.Seek(dwPos)) {
		rmf4.Close();
		return FALSE;
	}

	DWORD dwSize = rmf4.Read(RecLen * n, ptr);
	rmf4.Close();

	return TRUE;
}