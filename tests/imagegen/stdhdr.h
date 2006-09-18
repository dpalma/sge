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

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_STDHDR_H)
