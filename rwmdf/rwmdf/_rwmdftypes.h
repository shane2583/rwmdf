#pragma once

#include "stdafx.h"
#include <vector>
#include "mdf4.h"


using namespace std;


#ifndef WIN32
typedef unsigned long DWORD;
typedef long long __int64;
#endif



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


class DG;
class CG;

class CN {
public:
	CN(M4CNBlock *_cn) {
		m_block = _cn;
	}
	~CN() {
		//clear();
	}

	M4CNBlock* get_block() {
		return m_block;
	}

	CG* get_cg() const {
		return m_cg;
	}

	void set_cg(CG* _cg) {
		m_cg = _cg;
	}


protected:
private:
	M4CNBlock	*m_block;
	CG			*m_cg;

	void clear() {
		if (m_block) {
			delete m_block;
			m_block = nullptr;
		}
	}
};


class CG {

	typedef	vector<CN*>	ary_cns;

public:
	CG(M4CGBlock *_cg) {
		m_block = _cg;
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
	void set_dg(DG* _dg) {
		m_dg = _dg;
	}

	void add_cn(CN* _cn) {
		if (_cn) {
			_cn->set_cg(this);
			m_cns.push_back(_cn);
		}
	}


protected:
private:
	M4CGBlock	*m_block;
	DG*			m_dg;
	ary_cns		m_cns;

	void clear() {
		//if (m_block) {
		//	delete m_block;
		//	m_block = nullptr;
		//}
		for (auto &cn : m_cns) {
			if (cn) {
				delete cn;
				cn = nullptr;
			}
		}
		m_cns.clear();
	}
};


class DG {
	
	typedef vector<CG*>	ary_cgs;

public:
	DG(M4DGBlock* _dg) {
		m_block = _dg;
	}
	~DG() {
		clear();
	}

	M4DGBlock* get_dg() const {
		return m_block;
	}

	void add_cg(CG* _cg) {
		if (_cg) {
			_cg->set_dg(this);
			m_cgs.push_back(_cg);
		}
	}

protected:
private:
	M4DGBlock * m_block;
	ary_cgs		m_cgs;

	void clear() {
		//if (m_block) {
		//	delete m_block;
		//	m_block = nullptr;
		//}
		for (auto &cg : m_cgs) {
			if (cg) {
				delete cg;
				cg = nullptr;
			}	
		}
		m_cgs.clear();
	}
};

