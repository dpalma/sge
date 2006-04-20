/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORAPP_H)
#define INCLUDED_EDITORAPP_H

#include "editorapi.h"
#include "afxcomtools.h"

#include "globalobjdef.h"

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
// CLASS: cEditorSingleDocTemplate
//

class cEditorSingleDocTemplate : public CSingleDocTemplate
{
   cEditorSingleDocTemplate(const cEditorSingleDocTemplate &);
   void operator =(const cEditorSingleDocTemplate &);

public:
   cEditorSingleDocTemplate(UINT nIDResource, CRuntimeClass * pDocClass,
      CRuntimeClass * pFrameClass, CRuntimeClass * pViewClass);
   ~cEditorSingleDocTemplate();
};

////////////////////////////////////////

inline cEditorSingleDocTemplate::cEditorSingleDocTemplate(UINT nIDResource,
                                                          CRuntimeClass * pDocClass,
                                                          CRuntimeClass * pFrameClass,
                                                          CRuntimeClass * pViewClass)
 : CSingleDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{
}

////////////////////////////////////////

inline cEditorSingleDocTemplate::~cEditorSingleDocTemplate()
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorApp
//

class cEditorApp : public CWinApp, 
                   public cComObject2<IMPLEMENTS(IEditorApp), IMPLEMENTS(IGlobalObject), cAfxComServices<cEditorApp> >
{
public:
	cEditorApp();
	~cEditorApp();

   DECLARE_NAME(EditorApp)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
   virtual void ParseCommandLine(CCommandLineInfo& rCmdInfo);

// Implementation
	//{{AFX_MSG(cEditorApp)
	afx_msg void OnAppAbout();
	afx_msg void OnToolsUnitTestRunner();
	afx_msg void OnToolsExecuteScriptCommand();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   virtual tResult AddEditorAppListener(IEditorAppListener * pListener);
   virtual tResult RemoveEditorAppListener(IEditorAppListener * pListener);

   virtual tResult GetActiveModel(IEditorModel * * ppModel);

   virtual tResult SetDefaultTileSet(const tChar * pszTileSet);
   virtual tResult GetDefaultTileSet(cStr * pTileSet) const;

private:
   typedef std::vector<IEditorAppListener *> tEditorAppListeners;
   tEditorAppListeners m_editorAppListeners;

   cStr m_defaultTileSet;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORAPP_H)
