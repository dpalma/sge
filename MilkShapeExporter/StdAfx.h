/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_STDAFX_H
#define INCLUDED_STDAFX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Include <afx.h> first because it has pragmas to link
// the MFC and CRT libraries in the correct order
#include <afx.h>

#include "techtypes.h"
#include "techdebug.h"
#include "combase.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>

#endif // !INCLUDED_STDAFX_H
