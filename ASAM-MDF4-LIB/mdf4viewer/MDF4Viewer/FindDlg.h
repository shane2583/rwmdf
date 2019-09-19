#if !defined(AFX_FINDDLG_H__D478D83D_6C93_4DA0_B2F0_34334B611798__INCLUDED_)
#define AFX_FINDDLG_H__D478D83D_6C93_4DA0_B2F0_34334B611798__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog

class CFindDlg : public CDialog
{
// Construction
public:
	CFindDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindDlg)
	enum { IDD = IDD_PRJFINDDLG };
	BOOL	m_bCase;
	CString	m_strText;
	BOOL	m_bWord;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bSearchAll;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDDLG_H__D478D83D_6C93_4DA0_B2F0_34334B611798__INCLUDED_)
