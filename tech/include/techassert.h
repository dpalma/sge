///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHASSERT_H
#define INCLUDED_TECHASSERT_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

#ifdef _DEBUG
#define ASSERTIONS_ENABLED
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(_MANAGED)
extern "C" DECLSPEC_DLLIMPORT void STDCALL DebugBreak();
#define DbgBreak     DebugBreak
#elif defined(_MSC_VER)
#define DbgBreak()   __asm { int 3h }
#elif defined(__GNUC__)
#define DbgBreak()   asm("int $3")
#else
#error ("Need compiler-specific definition of DbgBreak")
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
extern "C" DECLSPEC_DLLIMPORT int STDCALL IsDebuggerPresent();
#else
#define IsDebuggerPresent() (false)
#endif

///////////////////////////////////////////////////////////////////////////////
// Assert & Verify macros

#ifdef ASSERTIONS_ENABLED
#define AssertOnce(expr) \
   static class MAKE_UNIQUE(cAssertOnce) { \
   public: MAKE_UNIQUE(cAssertOnce)() { \
      Assert(expr); \
   }  } MAKE_UNIQUE(g_assertOnce);
#define Assert(expr)          AssertMsg(expr, NULL)
#define AssertMsg(expr,msg)   do { if (!(expr)) { if (AssertFail(_T(__FILE__), __LINE__, _T(#expr), msg)) DbgBreak(); } } while(0)
#define Verify(expr)          Assert(expr)
#else
#define AssertOnce(expr)
#define Assert(expr)          ((void)0)
#define AssertMsg(expr,msg)   ((void)0)
#define Verify(expr)          (expr)
#endif

///////////////////////////////////////////////////////////////////////////////

bool AssertFail(const tChar * pszFile, int line, const tChar * pszExpr, const tChar * pszMsg);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHASSERT_H
