#pragma once

#include "MDF4Viewer.h"
#include <vector>
#include "mdf4.h"

typedef struct {                 /* BitFeld                      */
  unsigned gaps:2;       /*   0: normal, 1: remove gaps, 2: duplicate files  */
  unsigned extendrange:1;/*   min/max + 10% */
} MDF4Flags;

class CReadFile;
class CMdf4FileImport;
class CMdf4DataGroup;


class CReadFileMF4
{
public:
	CReadFileMF4()
	{
		m_pDataStream = NULL;
		m_dwBufferSize = 0;
		m_iBufCtr = 0;
		m_nBuf = 0;
		m_iBufSiz = 0;
		m_pBuffer = NULL;
	}
	~CReadFileMF4()
	{
		if (m_pDataStream)
			delete m_pDataStream;
	}

	BOOL Open(M4DGBlock *pdg, int iBufferSize, int iBufCnt)
	{
		m_iBufSiz = iBufferSize;
		m_dwBufferSize = iBufferSize * iBufCnt;
		m_pBuffer = calloc(1,m_dwBufferSize);
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
	int Seek( __int64 i64Pos )
	{
		m_iBufCtr = 0;
		if (m_pDataStream == NULL)
			return 0;
		return m_pDataStream->Seek(i64Pos) == i64Pos;
	}
	DWORD Read( DWORD dwSize, void *pBuffer)
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

class CMdf4Calc
{
public:
	CMdf4Calc()
	{
		m_cn_data_type = 0;
		m_pCC = NULL;
		m_bCCalloced = FALSE;
	}
	CMdf4Calc(M4CNBlock *pChan, M4CCBlock *pCC)
	{
		m_cn_data_type = pChan->cn_data_type;
		m_pCC = pCC;
		m_bCCalloced = FALSE;
	}
	CMdf4Calc(M4CNBlock *pChan, MDF4File &m4)
	{
		m_cn_data_type = pChan->cn_data_type;
		m_pCC = (M4CCBlock *)m4.LoadLink(*pChan, M4CNBlock::cn_cc_conversion);
		m_bCCalloced = TRUE;
	}
	~CMdf4Calc()
	{
		if (m_pCC && m_bCCalloced)
			delete m_pCC;
	}
	
	M4CCBlock *m_pCC;
	BOOL       m_bCCalloced;
	M_UINT8 m_cn_data_type;

	double MdfCalc(double val);
	double do_tab(double val);
	double do_tabi(double val);
	double do_RangeToValue(double val, BOOL bIsInt = FALSE);

};

class CMdf4DataGroup
{
public:
	CMdf4DataGroup(void)
	{
		m_iGrp = 0;
		m_pDataStream = NULL;
		m_pRF = NULL;
		m_dg = NULL;
		m_pMdf = NULL;
	}
	~CMdf4DataGroup(void)
	{
		m_pMdf = NULL; // nicht freigeben!
		if (m_pDataStream)
			delete m_pDataStream;
		if (m_pRF)
			delete m_pRF;
		if (m_dg)
			delete m_dg;
	}
	CMdf4FileImport *m_pMdf;
	M4DGBlock *m_dg; // ptr auf Data Group block
	int m_iGrp; // group no., 1-N
  int m_iAllGrp; // group no. of all DGs, 1-N
  m4DataStream *m_pDataStream;
	CReadFileMF4 *m_pRF;

	BOOL GetRecord(M4CGBlock *cg, BYTE *ptr, M_UINT64 ix1, M_UINT64 ix2);
	BOOL GetRawValueFromRecord(M4CGBlock *cg, M4CNBlock *cn, BYTE *pRecord, double *pValue);
	double GetValueFromRecord(BYTE *dptr, int sbit, int nbit, int nbytes, BOOL bInteger, BOOL bSigned, BOOL bMotorola, BOOL bFloat);

};

class CMdf4FileImport
{
public:
	CMdf4FileImport(void);
	~CMdf4FileImport(void);

	// Helpers
	CString GetString(M4TXBlock *pTX);
	m4Block* LoadLink(m4Block *pParent,int linkNo);

  MDF4File m_m4;
	std::vector<CMdf4DataGroup*> m_vDataGroups;
	int m_nDataGroups;
	int m_nDataGroupsInvalid;
	CString m_strPathName; // the complete path name of the file
	FILETIME m_tmFileTime;
	long m_datum;

public:
	BOOL ImportFile(LPCTSTR szFileName, LPCTSTR szUser);
	void ReleaseFile(void);
};

