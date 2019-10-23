#pragma once

#include "stdafx.h"
#include "mdf4.h"


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

class CN {
public:
	CN() {

	}
	~CN() {

	}

protected:
private:
};


class CG {
public:
	CG() {

	}
	~CG() {

	}

protected:
private:
};

class DG {

public:
	DG() {

	}
	~DG() {

	}

protected:
private:
	
};
