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
	afx_msg void OnToolsUnitTestRunner();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   virtual tResult AddLoopClient(IEditorLoopClient * pLoopClient);
   virtual tResult RemoveLoopClient(IEditorLoopClient * pLoopClient);

   virtual tResult GetMapSettings(cMapSettings * pMapSettings);

private:
   typedef std::vector<IEditorLoopClient *> tEditorLoopClients;
   tEditorLoopClients m_loopClients;

   bool m_bPromptMapSettings;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORAPP_H)
