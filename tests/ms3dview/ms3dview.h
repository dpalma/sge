// ms3dview.h : main header file for the MS3DVIEW application
//

#if !defined(AFX_MS3DVIEW_H__13F759AF_83D0_4CA1_AD79_CA508A9530EB__INCLUDED_)
#define AFX_MS3DVIEW_H__13F759AF_83D0_4CA1_AD79_CA508A9530EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewApp:
// See ms3dview.cpp for the implementation of this class
//

class CMs3dviewApp : public CWinApp
{
public:
	CMs3dviewApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMs3dviewApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CMs3dviewApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MS3DVIEW_H__13F759AF_83D0_4CA1_AD79_CA508A9530EB__INCLUDED_)
