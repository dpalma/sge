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

#if defined(_MSC_VER)
#define DbgBreak()   do { __asm int 3h } while (0)
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
#define Assert(expr)          do { if (!(expr)) { if (AssertFail(__FILE__, __LINE__, #expr)) DbgBreak(); } } while(0)
#define AssertMsg(expr,msg)   do { if (!(expr)) { if (AssertFail(__FILE__, __LINE__, #expr "\n" msg)) DbgBreak(); } } while(0)
#define Verify(expr)          Assert(expr)
#else
#define AssertOnce(expr)
#define Assert(expr)          ((void)0)
#define AssertMsg(expr,msg)   ((void)0)
#define Verify(expr)          (expr)
#endif

///////////////////////////////////////////////////////////////////////////////

TECH_API bool AssertFail(const char * pszFile, int line, const char * pszExpr);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHASSERT_H
