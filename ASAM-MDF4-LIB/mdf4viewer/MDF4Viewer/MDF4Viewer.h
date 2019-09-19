
// MDF4Viewer.h: Hauptheaderdatei für die PROJECT_NAME-Anwendung
//

#pragma once

#ifndef __AFXWIN_H__
	#error "'stdafx.h' vor dieser Datei für PCH einschließen"
#endif

#include "resource.h"		// Hauptsymbole

void *mymemcpy(void *pDst, const void *pSrc, size_t nSize);

// CMDF4ViewerApp:
// Siehe MDF4Viewer.cpp für die Implementierung dieser Klasse
//

class CMDF4ViewerApp : public CWinApp
{
public:
	CMDF4ViewerApp();

// Überschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()
};

extern CMDF4ViewerApp theApp;