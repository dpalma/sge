/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_STDHDR_H)
#define INCLUDED_STDHDR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0500

#define QI_TEMPLATE_METHOD_FOR_ATL

#include "tech/techtypes.h"
#include "tech/techassert.h"
#include "tech/techlog.h"
#include "tech/combase.h"

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _ATL_USE_DDX_FLOAT

#define _WTL_NO_AUTOMATIC_NAMESPACE
#include <atlbase.h>
#include <atlapp.h>

extern WTL::CAppModule _Module;

#include <atlwin.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlframe.h>
#include <atlmisc.h>

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_STDHDR_H)
