/////////////////////////////////////////////////////////////////////////////
// $Id: editorApp.h 1262 2006-04-20 05:16:37Z dpalma $

#if !defined(INCLUDED_IGAPP_H)
#define INCLUDED_IGAPP_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenApp
//

class cImageGenApp : public CWinApp
{
public:
	cImageGenApp();
	~cImageGenApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cImageGenApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL
   virtual void ParseCommandLine(CCommandLineInfo& rCmdInfo);

// Implementation
	//{{AFX_MSG(cImageGenApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_IGAPP_H)
