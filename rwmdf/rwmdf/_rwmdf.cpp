#include "_rwmdf.h"


_RWMDF::_RWMDF()
{
	m_pHdr = nullptr;
}

_RWMDF::~_RWMDF()
{
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

long _RWMDF::get_dgroups(ary_dgs & dgs) {
	long ret = 0;

	if (0 < dgs.size()) {
		for (auto &dgblock : dgs) {
			if (dgblock)
				delete dgblock;
		}
		dgs.clear();
	}

	M4DGBlock *dg = (M4DGBlock*)m_m4.LoadLink(*m_pHdr, M4HDBlock::hd_dg_first);
	while (dg) {
		dgs.push_back(dg);
		dg = (M4DGBlock*)m_m4.LoadLink(*dg, M4DGBlock::dg_dg_next);
	}

	return dgs.size();
}