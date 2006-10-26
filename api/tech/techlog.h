///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHLOG_H
#define INCLUDED_TECHLOG_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLog
//

////////////////////////////////////////

enum eLogSeverity
{
   kError,
   kWarning,
   kInfo,
   kDebug,
};

////////////////////////////////////////

typedef void (* tLogCallbackFn)(eLogSeverity severity, const tChar * pszMsg, size_t msgLen);

////////////////////////////////////////

class TECH_API cLog
{
   cLog(const cLog &);
   const cLog & operator =(const cLog &);

public:
   cLog();
   ~cLog();

   void Initialize();
   void Shutdown();

   bool OpenLogFile(const cFileSpec & logFile);
   void CloseLogFile();

   bool DefineChannel(const tChar * pszChannel, bool * pEnableFlag);
   bool EnableChannel(const tChar * pszChannel, bool bEnable);

   void Print(const tChar * pszFile, int line, eLogSeverity severity, const tChar * pszFormat, ...);

   tLogCallbackFn SetCallback(tLogCallbackFn pfn);

private:
   bool * FindChannel(const tChar * pszChannel) const;

   class cLogChannel
   {
   public:
      cLogChannel(const tChar * pszName, bool * pbEnabled, cLogChannel * pNext);
      const tChar * GetName() const;
      bool * GetEnabledPointer() const;
      cLogChannel * GetNext() const;
   private:
      const tChar * m_pszName;
      bool * m_pbEnabled;
      cLogChannel * m_pNext;
   };

   cLogChannel * m_pChannels;

   tLogCallbackFn m_callback;

   enum
   {
      kBufferSize = 0x400
   };

   tChar m_szBuffer[kBufferSize];

   void * m_pLogFile;
};

////////////////////////////////////////

inline const tChar * cLog::cLogChannel::GetName() const
{
   return m_pszName;
}

////////////////////////////////////////

inline bool * cLog::cLogChannel::GetEnabledPointer() const
{
   return m_pbEnabled;
}

////////////////////////////////////////

inline cLog::cLogChannel * cLog::cLogChannel::GetNext() const
{
   return m_pNext;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogInitializer
//

class TECH_API cLogInitializer
{
   cLogInitializer(const cLogInitializer &);
   const cLogInitializer & operator =(const cLogInitializer &);

public:
   cLogInitializer();
   ~cLogInitializer();

private:
   static long gm_initCount;
};

///////////////////////////////////////////////////////////////////////////////

extern TECH_API cLog techlog;

///////////////////////////////////////////////////////////////////////////////

#define LogMsgNoFL(sev,fmt)                                 techlog.Print(NULL,0,(sev),_T(fmt))
#define LogMsgNoFL1(sev,fmt,arg1)                           techlog.Print(NULL,0,(sev),_T(fmt),(arg1))
#define LogMsgNoFL2(sev,fmt,arg1,arg2)                      techlog.Print(NULL,0,(sev),_T(fmt),(arg1),(arg2))
#define LogMsgNoFL3(sev,fmt,arg1,arg2,arg3)                 techlog.Print(NULL,0,(sev),_T(fmt),(arg1),(arg2),(arg3))
#define LogMsgNoFL4(sev,fmt,arg1,arg2,arg3,arg4)            techlog.Print(NULL,0,(sev),_T(fmt),(arg1),(arg2),(arg3),(arg4))
#define LogMsgNoFL5(sev,fmt,arg1,arg2,arg3,arg4,arg5)       techlog.Print(NULL,0,(sev),_T(fmt),(arg1),(arg2),(arg3),(arg4),(arg5))

#define LogMsg(sev,fmt)                                     techlog.Print(_T(__FILE__),__LINE__,(sev),_T(fmt))
#define LogMsg1(sev,fmt,arg1)                               techlog.Print(_T(__FILE__),__LINE__,(sev),_T(fmt),(arg1))
#define LogMsg2(sev,fmt,arg1,arg2)                          techlog.Print(_T(__FILE__),__LINE__,(sev),_T(fmt),(arg1),(arg2))
#define LogMsg3(sev,fmt,arg1,arg2,arg3)                     techlog.Print(_T(__FILE__),__LINE__,(sev),_T(fmt),(arg1),(arg2),(arg3))
#define LogMsg4(sev,fmt,arg1,arg2,arg3,arg4)                techlog.Print(_T(__FILE__),__LINE__,(sev),_T(fmt),(arg1),(arg2),(arg3),(arg4))
#define LogMsg5(sev,fmt,arg1,arg2,arg3,arg4,arg5)           techlog.Print(_T(__FILE__),__LINE__,(sev),_T(fmt),(arg1),(arg2),(arg3),(arg4),(arg5))

#define LogMsgIf(expr,sev,fmt)                              (void)((expr) && (LogMsg((sev),fmt), 1))
#define LogMsgIf1(expr,sev,fmt,arg1)                        (void)((expr) && (LogMsg1((sev),fmt,(arg1)), 1))
#define LogMsgIf2(expr,sev,fmt,arg1,arg2)                   (void)((expr) && (LogMsg2((sev),fmt,(arg1),(arg2)), 1))
#define LogMsgIf3(expr,sev,fmt,arg1,arg2,arg3)              (void)((expr) && (LogMsg3((sev),fmt,(arg1),(arg2),(arg3)), 1))
#define LogMsgIf4(expr,sev,fmt,arg1,arg2,arg3,arg4)         (void)((expr) && (LogMsg4((sev),fmt,(arg1),(arg2),(arg3),(arg4)), 1))
#define LogMsgIf5(expr,sev,fmt,arg1,arg2,arg3,arg4,arg5)    (void)((expr) && (LogMsg5((sev),fmt,(arg1),(arg2),(arg3),(arg4),(arg5)), 1))

#ifndef NDEBUG
#define DebugMsgIf(expr,fmt)                                LogMsgIf(expr,kDebug,fmt)
#define DebugMsgIf1(expr,fmt,arg1)                          LogMsgIf1(expr,kDebug,fmt,arg1)
#define DebugMsgIf2(expr,fmt,arg1,arg2)                     LogMsgIf2(expr,kDebug,fmt,arg1,arg2)
#define DebugMsgIf3(expr,fmt,arg1,arg2,arg3)                LogMsgIf3(expr,kDebug,fmt,arg1,arg2,arg3)
#define DebugMsgIf4(expr,fmt,arg1,arg2,arg3,arg4)           LogMsgIf4(expr,kDebug,fmt,arg1,arg2,arg3,arg4)
#define DebugMsgIf5(expr,fmt,arg1,arg2,arg3,arg4,arg5)      LogMsgIf5(expr,kDebug,fmt,arg1,arg2,arg3,arg4,arg5)
#else
#define DebugMsgIf(expr,fmt)                                ((void)0)
#define DebugMsgIf1(expr,fmt,arg1)                          ((void)0)
#define DebugMsgIf2(expr,fmt,arg1,arg2)                     ((void)0)
#define DebugMsgIf3(expr,fmt,arg1,arg2,arg3)                ((void)0)
#define DebugMsgIf4(expr,fmt,arg1,arg2,arg3,arg4)           ((void)0)
#define DebugMsgIf5(expr,fmt,arg1,arg2,arg3,arg4,arg5)      ((void)0)
#endif

#ifndef NDEBUG
#define DebugMsg(fmt)                                       LogMsg(kDebug,fmt)
#define DebugMsg1(fmt,arg1)                                 LogMsg1(kDebug,fmt,arg1)
#define DebugMsg2(fmt,arg1,arg2)                            LogMsg2(kDebug,fmt,arg1,arg2)
#define DebugMsg3(fmt,arg1,arg2,arg3)                       LogMsg3(kDebug,fmt,arg1,arg2,arg3)
#define DebugMsg4(fmt,arg1,arg2,arg3,arg4)                  LogMsg4(kDebug,fmt,arg1,arg2,arg3,arg4)
#define DebugMsg5(fmt,arg1,arg2,arg3,arg4,arg5)             LogMsg5(kDebug,fmt,arg1,arg2,arg3,arg4,arg5)
#else
#define DebugMsg(fmt)                                       ((void)0)
#define DebugMsg1(fmt,arg1)                                 ((void)0)
#define DebugMsg2(fmt,arg1,arg2)                            ((void)0)
#define DebugMsg3(fmt,arg1,arg2,arg3)                       ((void)0)
#define DebugMsg4(fmt,arg1,arg2,arg3,arg4)                  ((void)0)
#define DebugMsg5(fmt,arg1,arg2,arg3,arg4,arg5)             ((void)0)
#endif


#define ErrorMsgIf(expr,fmt)                                LogMsgIf(expr,kError,fmt)
#define ErrorMsgIf1(expr,fmt,arg1)                          LogMsgIf1(expr,kError,fmt,arg1)
#define ErrorMsgIf2(expr,fmt,arg1,arg2)                     LogMsgIf2(expr,kError,fmt,arg1,arg2)
#define ErrorMsgIf3(expr,fmt,arg1,arg2,arg3)                LogMsgIf3(expr,kError,fmt,arg1,arg2,arg3)
#define ErrorMsgIf4(expr,fmt,arg1,arg2,arg3,arg4)           LogMsgIf4(expr,kError,fmt,arg1,arg2,arg3,arg4)
#define ErrorMsgIf5(expr,fmt,arg1,arg2,arg3,arg4,arg5)      LogMsgIf5(expr,kError,fmt,arg1,arg2,arg3,arg4,arg5)

#define ErrorMsg(fmt)                                       LogMsg(kError,fmt)
#define ErrorMsg1(fmt,arg1)                                 LogMsg1(kError,fmt,arg1)
#define ErrorMsg2(fmt,arg1,arg2)                            LogMsg2(kError,fmt,arg1,arg2)
#define ErrorMsg3(fmt,arg1,arg2,arg3)                       LogMsg3(kError,fmt,arg1,arg2,arg3)
#define ErrorMsg4(fmt,arg1,arg2,arg3,arg4)                  LogMsg4(kError,fmt,arg1,arg2,arg3,arg4)
#define ErrorMsg5(fmt,arg1,arg2,arg3,arg4,arg5)             LogMsg5(kError,fmt,arg1,arg2,arg3,arg4,arg5)

#define WarnMsgIf(expr,fmt)                                 LogMsgIf(expr,kWarning,fmt)
#define WarnMsgIf1(expr,fmt,arg1)                           LogMsgIf1(expr,kWarning,fmt,arg1)
#define WarnMsgIf2(expr,fmt,arg1,arg2)                      LogMsgIf2(expr,kWarning,fmt,arg1,arg2)
#define WarnMsgIf3(expr,fmt,arg1,arg2,arg3)                 LogMsgIf3(expr,kWarning,fmt,arg1,arg2,arg3)
#define WarnMsgIf4(expr,fmt,arg1,arg2,arg3,arg4)            LogMsgIf4(expr,kWarning,fmt,arg1,arg2,arg3,arg4)
#define WarnMsgIf5(expr,fmt,arg1,arg2,arg3,arg4,arg5)       LogMsgIf5(expr,kWarning,fmt,arg1,arg2,arg3,arg4,arg5)

#define WarnMsg(fmt)                                        LogMsg(kWarning,fmt)
#define WarnMsg1(fmt,arg1)                                  LogMsg1(kWarning,fmt,arg1)
#define WarnMsg2(fmt,arg1,arg2)                             LogMsg2(kWarning,fmt,arg1,arg2)
#define WarnMsg3(fmt,arg1,arg2,arg3)                        LogMsg3(kWarning,fmt,arg1,arg2,arg3)
#define WarnMsg4(fmt,arg1,arg2,arg3,arg4)                   LogMsg4(kWarning,fmt,arg1,arg2,arg3,arg4)
#define WarnMsg5(fmt,arg1,arg2,arg3,arg4,arg5)              LogMsg5(kWarning,fmt,arg1,arg2,arg3,arg4,arg5)

#define InfoMsgIf(expr,fmt)                                 LogMsgIf(expr,kInfo,fmt)
#define InfoMsgIf1(expr,fmt,arg1)                           LogMsgIf1(expr,kInfo,fmt,arg1)
#define InfoMsgIf2(expr,fmt,arg1,arg2)                      LogMsgIf2(expr,kInfo,fmt,arg1,arg2)
#define InfoMsgIf3(expr,fmt,arg1,arg2,arg3)                 LogMsgIf3(expr,kInfo,fmt,arg1,arg2,arg3)
#define InfoMsgIf4(expr,fmt,arg1,arg2,arg3,arg4)            LogMsgIf4(expr,kInfo,fmt,arg1,arg2,arg3,arg4)
#define InfoMsgIf5(expr,fmt,arg1,arg2,arg3,arg4,arg5)       LogMsgIf5(expr,kInfo,fmt,arg1,arg2,arg3,arg4,arg5)

#define InfoMsg(fmt)                                        LogMsg(kInfo,fmt)
#define InfoMsg1(fmt,arg1)                                  LogMsg1(kInfo,fmt,arg1)
#define InfoMsg2(fmt,arg1,arg2)                             LogMsg2(kInfo,fmt,arg1,arg2)
#define InfoMsg3(fmt,arg1,arg2,arg3)                        LogMsg3(kInfo,fmt,arg1,arg2,arg3)
#define InfoMsg4(fmt,arg1,arg2,arg3,arg4)                   LogMsg4(kInfo,fmt,arg1,arg2,arg3,arg4)
#define InfoMsg5(fmt,arg1,arg2,arg3,arg4,arg5)              LogMsg5(kInfo,fmt,arg1,arg2,arg3,arg4,arg5)

///////////////////////////////////////////////////////////////////////////////
// Channeled debug output

#ifndef NDEBUG

#define LOG_DEFINE_ENABLE_CHANNEL(channel, enable) \
   bool g_bLogChannel##channel = (enable); \
   static cLogInitializer MAKE_UNIQUE(g_##channel##Initializer); \
   bool MAKE_UNIQUE(g_##channel##DefineResult) = techlog.DefineChannel(_T(#channel), &(g_bLogChannel##channel))

#define LOG_DEFINE_CHANNEL(channel) \
   LOG_DEFINE_ENABLE_CHANNEL(channel, false)

#define LOG_EXTERN_CHANNEL(channel) \
   extern bool g_bLogChannel##channel

#define LOG_ENABLE_CHANNEL(channel, enable) \
   techlog.EnableChannel(_T(#channel), (enable))

#define LOG_IS_CHANNEL_ENABLED(channel) \
   g_bLogChannel##channel

#else

#define LOG_DEFINE_ENABLE_CHANNEL(channel, enable)
#define LOG_DEFINE_CHANNEL(channel)
#define LOG_EXTERN_CHANNEL(channel)
#define LOG_ENABLE_CHANNEL(channel, enable)
#define LOG_IS_CHANNEL_ENABLED(channel) (false)

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

#endif // !INCLUDED_TECHLOG_H
