/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_SHADERTESTAPP_H)
#define INCLUDED_SHADERTESTAPP_H

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
// CLASS: cShaderTestApp
//

class cShaderTestApp : public CWinApp
{
public:
	cShaderTestApp();

   BOOL AddLoopClient(ShaderTest::cFrameLoopClient * pLoopClient);
   BOOL RemoveLoopClient(ShaderTest::cFrameLoopClient * pLoopClient);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cShaderTestApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(cShaderTestApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
   virtual int Run();

private:
   CArray<ShaderTest::cFrameLoopClient*, ShaderTest::cFrameLoopClient*> m_loopClients;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_SHADERTESTAPP_H)
