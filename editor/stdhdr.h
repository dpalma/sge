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

#define QI_TEMPLATE_METHOD_FOR_ATL

#include "techtypes.h"
#include "techdebug.h"
#include "combase.h"

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// The sizeable control bar files sometimes contain only "stdafx.h"
// so the headers are included here to minimize changing the (3rd party)
// source. See http://www.datamekanix.com/sizecbar/ for more info.
#include "sizecbar.h"
#include "scbarg.h"
#include "scbarcf.h"

#define _WTL_NO_AUTOMATIC_NAMESPACE
#include <atlbase.h>
#include <atlapp.h>

extern WTL::CAppModule _Module;

#include <atlwin.h>

#ifdef _DEBUG
#if _MSC_VER > 1200
#define DECLARE_DYNCREATE_EX(classname) \
   DECLARE_DYNCREATE(classname) \
   void * PASCAL operator new(size_t nSize, int type, LPCSTR lpszFileName, int nLine) { return ::operator new(nSize, type, lpszFileName, nLine); } \
   void PASCAL operator delete(void *p, int type, LPCSTR lpszFileName, int nLine) { ::operator delete(p); }
#else
#define DECLARE_DYNCREATE_EX(classname) \
   DECLARE_DYNCREATE(classname) \
   void * PASCAL operator new(size_t nSize, int type, LPCSTR lpszFileName, int nLine) { return ::operator new(nSize, type, lpszFileName, nLine); }
#endif
#define IMPLEMENT_DYNCREATE_EX(classname, baseclassname) \
   IMPLEMENT_DYNCREATE(classname, baseclassname)
#else
#define DECLARE_DYNCREATE_EX(classname) \
   DECLARE_DYNCREATE(classname)
#define IMPLEMENT_DYNCREATE_EX(classname, baseclassname) \
   IMPLEMENT_DYNCREATE(classname, baseclassname)
#endif

#pragma warning(disable:4355) // 'this' : used in base member initializer list

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_STDHDR_H)
