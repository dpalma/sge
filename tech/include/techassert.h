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
   inline void DbgBreak() { asm("int $3"); }
#else
   #define DbgBreak()      __asm { int 3h }
#endif

#ifdef _WIN32
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();
#else
#define IsDebuggerPresent() (FALSE)
#endif

///////////////////////////////////////////////////////////////////////////////
// Assert & verify macros

#ifdef _DEBUG
#define Assert(expr)    do { if (!(expr)) { if (AssertFail(__FILE__, __LINE__, #expr)) DbgBreak(); } } while(0)
#define AssertMsg(expr, msg) do { if (!(expr)) { if (AssertFail(__FILE__, __LINE__, #expr "\n" msg)) DbgBreak(); } } while(0)
#define Verify(expr)    Assert(expr)
#else
#define Assert(expr)    ((void)0)
#define AssertMsg(expr, msg) ((void)0)
#define Verify(expr)    (expr)
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
TECH_API void DebugPrintf(const char * pszFile, int line,
                          const char * pszFormat, ...);

TECH_API void DebugEchoFileStart(const char * pszFile);
TECH_API void DebugEchoFileStop();

#else

inline bool AssertFail(const char *, int, const char *) { return false; }
inline void DebugPrintf(const char *, int, const char *, ...) {}
#define DebugEchoFileStart(pszFile) ((void)0)
#define DebugEchoFileStop() ((void)0)

#endif


///////////////////////////////////////////////////////////////////////////////
// Debug output

#ifndef NDEBUG
#define DebugMsgIf(expr,fmt)                                do { if (expr) { DebugPrintf(__FILE__,__LINE__,fmt); } } while(0)
#define DebugMsgIf1(expr,fmt,arg1)                          do { if (expr) { DebugPrintf(__FILE__,__LINE__,fmt,(arg1)); } } while(0)
#define DebugMsgIf2(expr,fmt,arg1,arg2)                     do { if (expr) { DebugPrintf(__FILE__,__LINE__,fmt,(arg1),(arg2)); } } while(0)
#define DebugMsgIf3(expr,fmt,arg1,arg2,arg3)                do { if (expr) { DebugPrintf(__FILE__,__LINE__,fmt,(arg1),(arg2),(arg3)); } } while(0)
#define DebugMsgIf4(expr,fmt,arg1,arg2,arg3,arg4)           do { if (expr) { DebugPrintf(__FILE__,__LINE__,fmt,(arg1),(arg2),(arg3),(arg4)); } } while(0)
#define DebugMsgIf5(expr,fmt,arg1,arg2,arg3,arg4,arg5)      do { if (expr) { DebugPrintf(__FILE__,__LINE__,fmt,(arg1),(arg2),(arg3),(arg4),(arg5)); } } while(0)
#else
#define DebugMsgIf(expr,fmt)                                ((void)0)
#define DebugMsgIf1(expr,fmt,arg1)                          ((void)0)
#define DebugMsgIf2(expr,fmt,arg1,arg2)                     ((void)0)
#define DebugMsgIf3(expr,fmt,arg1,arg2,arg3)                ((void)0)
#define DebugMsgIf4(expr,fmt,arg1,arg2,arg3,arg4)           ((void)0)
#define DebugMsgIf5(expr,fmt,arg1,arg2,arg3,arg4,arg5)      ((void)0)
#endif

#ifndef NDEBUG
#define DebugMsg(fmt)                                       DebugMsgIf(1,fmt)
#define DebugMsg1(fmt,arg1)                                 DebugMsgIf1(1,fmt,arg1)
#define DebugMsg2(fmt,arg1,arg2)                            DebugMsgIf2(1,fmt,arg1,arg2)
#define DebugMsg3(fmt,arg1,arg2,arg3)                       DebugMsgIf3(1,fmt,arg1,arg2,arg3)
#define DebugMsg4(fmt,arg1,arg2,arg3,arg4)                  DebugMsgIf4(1,fmt,arg1,arg2,arg3,arg4)
#define DebugMsg5(fmt,arg1,arg2,arg3,arg4,arg5)             DebugMsgIf5(1,fmt,arg1,arg2,arg3,arg4,arg5)
#else
#define DebugMsg(fmt)                                       ((void)0)
#define DebugMsg1(fmt,arg1)                                 ((void)0)
#define DebugMsg2(fmt,arg1,arg2)                            ((void)0)
#define DebugMsg3(fmt,arg1,arg2,arg3)                       ((void)0)
#define DebugMsg4(fmt,arg1,arg2,arg3,arg4)                  ((void)0)
#define DebugMsg5(fmt,arg1,arg2,arg3,arg4,arg5)             ((void)0)
#endif

///////////////////////////////////////////////////////////////////////////////
// Channeled debug output

#ifndef NDEBUG

TECH_API bool LogDefineChannel(const char * pszLogChannel, bool * pbEnable);
TECH_API void LogEnableChannel(const char * pszLogChannel, bool bEnabled);

#define LOG_DEFINE_ENABLE_CHANNEL(channel, enable) \
   bool g_bLogChannel##channel = (enable); \
   bool MAKE_UNIQUE(ignore##channel) = LogDefineChannel(#channel, &(g_bLogChannel##channel))

#define LOG_DEFINE_CHANNEL(channel) \
   LOG_DEFINE_ENABLE_CHANNEL(channel, false)

#define LOG_EXTERN_CHANNEL(channel) \
   extern bool g_bLogChannel##channel

#define LOG_ENABLE_CHANNEL(channel, enable) \
   LogEnableChannel(#channel, (enable))

#define LOG_IS_CHANNEL_ENABLED(channel) \
   g_bLogChannel##channel

#else

inline bool LogDefineChannel(const char * pszLogChannel, bool * pbEnable) { return false; }
inline void LogEnableChannel(const char * pszLogChannel, bool bEnable) {}

#define LOG_DEFINE_ENABLE_CHANNEL(channel, enable)
#define LOG_DEFINE_CHANNEL(channel)
#define LOG_EXTERN_CHANNEL(channel)
#define LOG_ENABLE_CHANNEL(channel, enable)
#define LOG_IS_CHANNEL_ENABLED(channel)

#endif

#define DebugMsgIfEx(channel,expr,fmt)                            DebugMsgIf(LOG_IS_CHANNEL_ENABLED(channel)&&(expr),fmt)
#define DebugMsgIfEx1(channel,expr,fmt,arg1)                      DebugMsgIf1(LOG_IS_CHANNEL_ENABLED(channel)&&(expr),fmt,arg1)
#define DebugMsgIfEx2(channel,expr,fmt,arg1,arg2)                 DebugMsgIf2(LOG_IS_CHANNEL_ENABLED(channel)&&(expr),fmt,arg1,arg2)
#define DebugMsgIfEx3(channel,expr,fmt,arg1,arg2,arg3)            DebugMsgIf3(LOG_IS_CHANNEL_ENABLED(channel)&&(expr),fmt,arg1,arg2,arg3)
#define DebugMsgIfEx4(channel,expr,fmt,arg1,arg2,arg3,arg4)       DebugMsgIf4(LOG_IS_CHANNEL_ENABLED(channel)&&(expr),fmt,arg1,arg2,arg3,arg4)
#define DebugMsgIfEx5(channel,expr,fmt,arg1,arg2,arg3,arg4,arg5)  DebugMsgIf5(LOG_IS_CHANNEL_ENABLED(channel)&&(expr),fmt,arg1,arg2,arg3,arg4,arg5)

#define DebugMsgEx(channel,fmt)                             DebugMsgIf(LOG_IS_CHANNEL_ENABLED(channel),fmt)
#define DebugMsgEx1(channel,fmt,arg1)                       DebugMsgIf1(LOG_IS_CHANNEL_ENABLED(channel),fmt,arg1)
#define DebugMsgEx2(channel,fmt,arg1,arg2)                  DebugMsgIf2(LOG_IS_CHANNEL_ENABLED(channel),fmt,arg1,arg2)
#define DebugMsgEx3(channel,fmt,arg1,arg2,arg3)             DebugMsgIf3(LOG_IS_CHANNEL_ENABLED(channel),fmt,arg1,arg2,arg3)
#define DebugMsgEx4(channel,fmt,arg1,arg2,arg3,arg4)        DebugMsgIf4(LOG_IS_CHANNEL_ENABLED(channel),fmt,arg1,arg2,arg3,arg4)
#define DebugMsgEx5(channel,fmt,arg1,arg2,arg3,arg4,arg5)   DebugMsgIf5(LOG_IS_CHANNEL_ENABLED(channel),fmt,arg1,arg2,arg3,arg4,arg5)

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHDEBUG_H
