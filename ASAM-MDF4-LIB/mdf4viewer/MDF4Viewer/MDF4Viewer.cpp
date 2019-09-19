
// MDF4Viewer.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "MDF4Viewer.h"
#include "ImportMdf4.h"
#include "FileInfo4.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void *mymemcpy(void *pDst, const void *pSrc, size_t nSize)
{
	void *pRet = NULL;
	__try
	{
	  pRet = memcpy( pDst, pSrc, nSize);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("memcpy failed\n");
	}
	return pRet;
}

// CMDF4ViewerApp

BEGIN_MESSAGE_MAP(CMDF4ViewerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMDF4ViewerApp-Erstellung

CMDF4ViewerApp::CMDF4ViewerApp()
{
	// TODO: Hier Code zur Konstruktion einfügen
	// Alle wichtigen Initialisierungen in InitInstance positionieren
}


// Das einzige CMDF4ViewerApp-Objekt

CMDF4ViewerApp theApp;


// CMDF4ViewerApp-Initialisierung

BOOL CMDF4ViewerApp::InitInstance()
{
	// InitCommonControlsEx() ist für Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder höher zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Legen Sie dies fest, um alle allgemeinen Steuerelementklassen einzubeziehen,
	// die Sie in Ihrer Anwendung verwenden möchten.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();



	// Standardinitialisierung
	// Wenn Sie diese Features nicht verwenden und die Größe
	// der ausführbaren Datei verringern möchten, entfernen Sie
	// die nicht erforderlichen Initialisierungsroutinen.
	// Ändern Sie den Registrierungsschlüssel, unter dem Ihre Einstellungen gespeichert sind.
	// TODO: Ändern Sie diese Zeichenfolge entsprechend,
	// z.B. zum Namen Ihrer Firma oder Organisation.
	SetRegistryKey(_T("Mdf4Viewer"));

  static LPCTSTR fileFilter=_T("MDF4 Files (*.mf4)|*.mf4||");
  CFileDialog fd(TRUE,NULL,NULL,6UL|OFN_ENABLESIZING,fileFilter);
	// Get file name from user
  if (fd.DoModal()==IDOK)
  {
		CString strFileName = fd.GetPathName();

		CMdf4FileImport *pImport = new CMdf4FileImport;
		if (pImport->ImportFile(strFileName, NULL))
		{
			CFileInfo4 dlg;
			m_pMainWnd = &dlg;
			dlg.m_pMDF = pImport;
			INT_PTR nResponse = dlg.DoModal();
			if (nResponse == IDOK)
			{
			}
			else if (nResponse == IDCANCEL)
			{
			}
		}
		delete pImport;
	}

	return FALSE;
}

