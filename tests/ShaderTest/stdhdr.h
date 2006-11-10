// stdhdr.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(INCLUDED_STDHDR_H)
#define INCLUDED_STDHDR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// Use the inline function versions of these from <combase.h>
#undef SUCCEEDED
#undef FAILED

#include "tech/techtypes.h"
#include "tech/techassert.h"
#include "tech/techlog.h"

namespace ShaderTest
{
   class cFrameLoopClient
   {
   public:
      virtual ~cFrameLoopClient() = 0;
      virtual void OnFrame(double time, double elapsed) = 0;
   };
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_STDHDR_H)
