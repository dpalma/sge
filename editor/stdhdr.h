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

#include "techtypes.h"
#include "techdebug.h"

#define QI_TEMPLATE_METHOD_FOR_ATL

#include "combase.h"

#define _WTL_USE_CSTRING

#include <atlbase.h>
#include <atlapp.h>

extern WTL::CAppModule _Module;

#include <atlcom.h>
#include <atlwin.h>
#include <atlgdi.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlcrack.h>
#include <atlframe.h>

#pragma warning(disable:4355) // 'this' : used in base member initializer list

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_STDHDR_H)
