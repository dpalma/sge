/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(AFX_MS3DVIEW_H__13F759AF_83D0_4CA1_AD79_CA508A9530EB__INCLUDED_)
#define AFX_MS3DVIEW_H__13F759AF_83D0_4CA1_AD79_CA508A9530EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <afxtempl.h>

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dviewApp
//

class cMs3dviewApp : public CWinApp
{
public:
	cMs3dviewApp();

   BOOL AddLoopClient(ms3dview::cFrameLoopClient * pLoopClient);
   BOOL RemoveLoopClient(ms3dview::cFrameLoopClient * pLoopClient);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cMs3dviewApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(cMs3dviewApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
   virtual int Run();

private:
   CArray<ms3dview::cFrameLoopClient*, ms3dview::cFrameLoopClient*> m_loopClients;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MS3DVIEW_H__13F759AF_83D0_4CA1_AD79_CA508A9530EB__INCLUDED_)
