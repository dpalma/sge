/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_STDHDR_H)
#define INCLUDED_STDHDR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WINVER 0x0400
#define _WIN32_WINNT 0x0400
#define _WIN32_IE 0x0400

// Include <afx.h> first because it has pragmas to link
// the MFC and CRT libraries in the correct order
#include <afx.h>

#define QI_TEMPLATE_METHOD_FOR_ATL

#include "techtypes.h"
#include "techassert.h"
#include "techlog.h"
#include "combase.h"

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#if _MSC_VER < 1400
typedef UINT      NCHITTESTRESULT;
#else
typedef LRESULT   NCHITTESTRESULT;
#endif

// The sizeable control bar files sometimes contain only "stdafx.h"
// so the headers are included here to minimize changing the (3rd party)
// source. See http://www.datamekanix.com/sizecbar/ for more info.
#include "sizecbar.h"
#include "scbarg.h"
#include "scbarcf.h"

class cEditorControlBar : public CSizingControlBarG
{
   DECLARE_DYNCREATE(cEditorControlBar)
};

#define _WTL_NO_AUTOMATIC_NAMESPACE
#include <atlbase.h>
#include <atlapp.h>

extern WTL::CAppModule _Module;

#include <atlwin.h>

#ifndef _AFX_CLIENT_BLOCK
#define _AFX_CLIENT_BLOCK _CLIENT_BLOCK
#endif

#define DECLARE_DYNCREATE_EX(classname) \
   DECLARE_DYNCREATE(classname)
#define IMPLEMENT_DYNCREATE_EX(classname, baseclassname) \
   IMPLEMENT_DYNCREATE(classname, baseclassname)

#define NO_COPY(classname) \
   classname(const classname &); \
   const classname & operator =(const classname &)

#pragma warning(disable:4355) // 'this' : used in base member initializer list

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_STDHDR_H)
