
// FileInfo4.h: Headerdatei
//

#pragma once
#include "mdf4.h"
#include "ImportMdf4.h"

#define MAXIDLEN0 256

typedef struct SIgnal {
  struct SIgnal *next;	    /* für QUEUE-Verwaltung	*/
  UINT kennung,nummer;      /* für verschiedenes...	*/
  char name[MAXIDLEN0];	    /* Name des Signals         */
  struct {              /* BitFeld                      */
	  unsigned bLoaded:1;          // Daten geladen?
	  unsigned bIsTime:1;          // X-Signal (Zeit)
    unsigned bHasMiss:1;         // Wenn noval gültig ist (auch 0.0)
	  unsigned bMonotonSteigend:1; // für X-Signale
	  unsigned bIsCloned:1;        // Signal ist ein Klon
	  unsigned bInvalBits:1;       // Signal hat invalid bits (geladen)
		unsigned bHasSRBlock:1;      // SR-Block available?
	  unsigned ignore:2;           // >0, wenn ignorieren 
	  unsigned timeerr:1;          // >0, wenn Fehler im Zeitsignal 
	  unsigned readerr:1;          // >0, wenn Fehler beim Lesen (MDF4)
		unsigned allmissing:2;       // 0=weiß nicht, 1 = nein, 2 = ja, 3 = es gibt welche
    } flags;
	BYTE *pInvalidBits;
  void *memory;             /* falls Datei im Speicher  */
	int iGroup;
  __int64  anzahl;          /* Anzahl Werte vorhanden   */
  __int64  block0;
	double noval;             // Missing Value code 
	M4DGBlock *m_dg;
	M4CGBlock *m_cg;
	M4CNBlock *m_cn;
  __int64 dwOffsetAsDAFF;     // Offset, wenn es eine DAFF Datei wäre
  __int64 dwEndOffset;
} Signal,*SIGNAL;

class CFileInfo4; // forward

/////////////////////////////////////////////////////////////////////////////
// CInfoTree window

class CInfo4Tree : public CTreeCtrl
{
// Construction
public:
	CInfo4Tree();

// Attributes
public:
  BOOL m_bCase;
  BOOL m_bWords;
  UINT m_nFindIdx;
  UINT m_nLastFindIdx;
  UINT m_iScanSize;
  unsigned __int64 m_i64LastScanIndex;
  BYTE *m_pScan;
	void *m_pGroupEntryPtr;
	BOOL m_bStreamAllocated;

// Operations
public:
  BOOL Compare(CString strText, CString strTarget);
  HTREEITEM find_child(HTREEITEM p, const TCHAR *name);
	HTREEITEM find(HTREEITEM hInit, const TCHAR *name);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoTree)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CInfo4Tree();
	void CleanUp(HTREEITEM hParent);
	void DisplayM4Block( CFileInfo4 *pDlg, m4Block &block, int idx);
	void DisplayIDBlock(CFileInfo4 *pDlg, mdfFileId *pId);
	void DisplayHdrBlock(CFileInfo4 *pDlg, M4HDBlock *pHdr);
	void DisplayDGBlock(CFileInfo4 *pDlg, M4DGBlock *pDG);
	void DisplayCGBlock(CFileInfo4 *pDlg, M4CGBlock *pCG);
	void DisplaySRBlock(CFileInfo4 *pDlg, M4SRBlock *pSR);
	void DisplayCNBlock(CFileInfo4 *pDlg, M4CNBlock *pCN);
	void DisplayCCBlock(CFileInfo4 *pDlg, M4CCBlock *pCC);
	void DisplayEVBlock(CFileInfo4 *pDlg, M4EVBlock *pEV);
	void DisplayFHBlock(CFileInfo4 *pDlg, M4FHBlock *pFH);
	void DisplayHLBlock(CFileInfo4 *pDlg, M4HLBlock *pHL);
	void DisplayXML(CFileInfo4 *pDlg, LPCTSTR pszXML);	
	void DisplayDTBlock(CFileInfo4 *pDlg, M4DTBlock *pDT);
	void DisplayRDBlock(CFileInfo4 *pDlg, M4RDBlock *pRD);
	void DisplayDLBlock(CFileInfo4 *pDlg, M4DLBlock *pDL);
	void DisplayDZBlock(CFileInfo4 *pDlg, M4DZBlock *pDZ);
	void DisplayCABlock(CFileInfo4 *pDlg, M4CABlock *pCA);
	void DisplayData(CFileInfo4 *pDlg, M4DGBlock *pDG);
	void PrepareData(CListCtrl& List, CTreeCtrl& Tree, void* ptr);
	BOOL GetScan(unsigned __int64 i64Index, void *ptr);
	CString Show(int iItem, int iSubItem, BOOL bPhysVal);
	void Export();
	void FillXMLTree(CTreeCtrl &Tree, LPCTSTR pszXML);
	void InsertXMLNode(CTreeCtrl &Tree, HTREEITEM hParent, MSXML2::IXMLDOMNodePtr pParentNode);
	CMdf4DataGroup* FindDataGroup(std::vector<CMdf4DataGroup*> &vDataGroups, int ithGroup);

	// Generated message map functions
protected:
	//{{AFX_MSG(CInfoTree)
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRClick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

// CFileInfo4-Dialogfeld
class CFileInfo4 : public CDialogEx
{
// Konstruktion
public:
	CFileInfo4(CWnd* pParent = NULL);	// Standardkonstruktor
	virtual ~CFileInfo4();

// Dialogfelddaten
	enum { IDD = IDD_MDF4VIEWER_DIALOG };
	CInfo4Tree	m_Tree;
	CWnd*       m_pCurCtrl;
	CListCtrl	  m_List;
	CListCtrl	  m_DataList;
  CTreeCtrl   m_XMLTree;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	// Generated message map functions
	//{{AFX_MSG(CFileInfo)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFind();
	afx_msg void OnFindNext();
	afx_msg void OnGetDispInfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnExport();
	afx_msg void OnColumnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Implementierung
protected:
	HICON m_hIcon;

	// Generierte Funktionen für die Meldungstabellen
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	
public:
	void FillTree(HTREEITEM hParent);
	void AddDataBlocks( M_LINK at, HTREEITEM hParent);

  CMdf4FileImport* m_pMDF;
  HTREEITEM m_hItemLast;
  CString   m_strFindLast;
  BOOL      m_bShowingData;
  BOOL      m_bOneCol;
  CMdf4DataGroup* m_pCurDataGroup;
  TCHAR**    m_Cache;     
  DWORD     m_dwInCache;
  UINT      m_nChan;
	virtual void OnOK();
	virtual void OnCancel();
	BOOL m_bPhysVal;
	afx_msg void OnBnClickedPhys();
	afx_msg void OnTvnItemExpandedTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemExpandingTree(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
};
