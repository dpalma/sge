/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORAPP_H)
#define INCLUDED_EDITORAPP_H

#include "editorapi.h"
#include "afxcomtools.h"

#include <afxdisp.h>

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorApp
//

class cEditorApp : public CWinApp, 
                   public cComObject<IMPLEMENTS(IEditorApp), cAfxComServices<cEditorApp> >
{
public:
	cEditorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(cEditorApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   tResult AddLoopClient(IEditorLoopClient * pLoopClient);
   tResult RemoveLoopClient(IEditorLoopClient * pLoopClient);

private:
   typedef std::vector<IEditorLoopClient *> tEditorLoopClients;
   tEditorLoopClients m_loopClients;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORAPP_H)
