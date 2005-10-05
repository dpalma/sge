///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHDEBUG_H
#define INCLUDED_TECHDEBUG_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
// Symbol | Debug | Release | Opt
//--------------------------------
// _DEBUG |   Y   |    N    |  Y
// NDEBUG |   N   |    Y    |  Y

///////////////////////////////////////////////////////////////////////////////

#if defined(__GNUC__)
inline void DbgBreak()  { asm("int $3"); }
#else
#define DbgBreak()      __asm { int 3h }
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
extern "C" DECLSPEC_DLLIMPORT int STDCALL IsDebuggerPresent();
#else
#define IsDebuggerPresent() (false)
#endif

///////////////////////////////////////////////////////////////////////////////
// Assert & verify macros

#ifdef _DEBUG
#define Assert(expr)          do { if (!(expr)) { if (AssertFail(__FILE__, __LINE__, #expr)) DbgBreak(); } } while(0)
#define AssertMsg(expr,msg)   do { if (!(expr)) { if (AssertFail(__FILE__, __LINE__, #expr "\n" msg)) DbgBreak(); } } while(0)
#define Verify(expr)          Assert(expr)
#else
#define Assert(expr)          ((void)0)
#define AssertMsg(expr,msg)   ((void)0)
#define Verify(expr)          (expr)
#endif

#ifdef _DEBUG

#define BeginAssertOnceImpl \
   class MAKE_UNIQUE(cAssertOnceImpl_) \
   { \
   public: \
      MAKE_UNIQUE(cAssertOnceImpl_)() \
      {

#define EndAssertOnceImpl \
      } \
   }; \
   static MAKE_UNIQUE(cAssertOnceImpl_) MAKE_UNIQUE(g_AssertOnce_)

#define AssertOnce(expr) \
   BeginAssertOnceImpl \
      Assert(expr); \
   EndAssertOnceImpl

#else

#define AssertOnce(expr)

#endif

///////////////////////////////////////////////////////////////////////////////
// Debug-only function prototypes

#ifdef _DEBUG
TECH_API bool AssertFail(const char * pszFile, int line, const char * pszExpr);
#else
inline bool AssertFail(const char *, int, const char *) { return false; }
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHDEBUG_H
