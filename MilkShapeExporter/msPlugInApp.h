/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MSPLUGINAPP_H
#define INCLUDED_MSPLUGINAPP_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMsPlugInApp
//

class cMsPlugInApp : public CWinApp
{
public:
	cMsPlugInApp();

	//{{AFX_VIRTUAL(cMsPlugInApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(cMsPlugInApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MSPLUGINAPP_H
