/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_STDHDR_H)
#define INCLUDED_STDHDR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#pragma warning(disable:4355) // 'this' : used in base member initializer list

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_STDHDR_H)
