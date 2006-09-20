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

bool AssertFail(const tChar * pszFile, int line, const tChar * pszExpr, const tChar * pszMsg);

///////////////////////////////////////////////////////////////////////////////
// Assert & Verify macros

#ifdef ASSERTIONS_ENABLED

class cAssert
{
public:
   cAssert(bool bResult, const tChar * pszFile, int line, const tChar * pszExpr, const tChar * pszMsg)
   {
      if (!bResult)
         if (AssertFail(pszFile, line, pszExpr, pszMsg))
            DbgBreak();
   }
};

#define AssertMsg(expr,msg) \
   do { if (!(expr)) { if (AssertFail(_T(__FILE__), __LINE__, _T(#expr), msg)) DbgBreak(); } } while(0)

#define Assert(expr) \
   AssertMsg(expr, NULL)

#define AssertOnce(expr) \
   static cAssert MAKE_UNIQUE(oneTimeAssertion)(expr, _T(__FILE__), __LINE__, #expr, NULL)

#define Verify(expr) \
   Assert(expr)

#else
#define AssertMsg(expr,msg)         ((void)0)
#define Assert(expr)                ((void)0)
#define AssertOnce(expr)            // AssertOnce should evaluate to nothing so it works at file scope
#define Verify(expr)                (expr)
#endif

// Compile-time assertions generate no runnable code, so they are
// enabled regardless of the definition of ASSERTIONS_ENABLED
#define AssertAtCompileTime(expr) \
   typedef char MAKE_UNIQUE(compileTimeAssertion)[expr]

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHASSERT_H
