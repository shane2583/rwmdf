// FindDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Mdf4Viewer.h"
#include "FindDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog


CFindDlg::CFindDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindDlg::IDD, pParent)
	, m_bSearchAll(FALSE)
{
	//{{AFX_DATA_INIT(CFindDlg)
	m_bCase = FALSE;
	m_strText = _T("");
	m_bWord = FALSE;
	m_bSearchAll = FALSE;
	//}}AFX_DATA_INIT
}


void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindDlg)
	DDX_Check(pDX, IDC_CASE, m_bCase);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	DDX_Check(pDX, IDC_WORD, m_bWord);
	DDX_Radio(pDX, IDC_ONLYSIG, m_bSearchAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindDlg, CDialog)
	//{{AFX_MSG_MAP(CFindDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindDlg message handlers
