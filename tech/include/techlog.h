///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHLOG_H
#define INCLUDED_TECHLOG_H

#include "techdll.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

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

   bool DefineChannel(const tChar * pszChannel, bool * pEnableFlag);
   bool EnableChannel(const tChar * pszChannel, bool bEnable);

   void Print(eLogSeverity severity, const tChar * pszFormat, ...);
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

#define ErrorMsgIf(expr,fmt)                                do { if (expr) { techlog.Print(__FILE__,__LINE__,kError,(fmt)); } } while(0)
#define ErrorMsgIf1(expr,fmt,arg1)                          do { if (expr) { techlog.Print(__FILE__,__LINE__,kError,(fmt),(arg1)); } } while(0)
#define ErrorMsgIf2(expr,fmt,arg1,arg2)                     do { if (expr) { techlog.Print(__FILE__,__LINE__,kError,(fmt),(arg1),(arg2)); } } while(0)
#define ErrorMsgIf3(expr,fmt,arg1,arg2,arg3)                do { if (expr) { techlog.Print(__FILE__,__LINE__,kError,(fmt),(arg1),(arg2),(arg3)); } } while(0)
#define ErrorMsgIf4(expr,fmt,arg1,arg2,arg3,arg4)           do { if (expr) { techlog.Print(__FILE__,__LINE__,kError,(fmt),(arg1),(arg2),(arg3),(arg4)); } } while(0)
#define ErrorMsgIf5(expr,fmt,arg1,arg2,arg3,arg4,arg5)      do { if (expr) { techlog.Print(__FILE__,__LINE__,kError,(fmt),(arg1),(arg2),(arg3),(arg4),(arg5)); } } while(0)

#define ErrorMsg(fmt)                                       ErrorMsgIf(1,fmt)
#define ErrorMsg1(fmt,arg1)                                 ErrorMsgIf1(1,fmt,arg1)
#define ErrorMsg2(fmt,arg1,arg2)                            ErrorMsgIf2(1,fmt,arg1,arg2)
#define ErrorMsg3(fmt,arg1,arg2,arg3)                       ErrorMsgIf3(1,fmt,arg1,arg2,arg3)
#define ErrorMsg4(fmt,arg1,arg2,arg3,arg4)                  ErrorMsgIf4(1,fmt,arg1,arg2,arg3,arg4)
#define ErrorMsg5(fmt,arg1,arg2,arg3,arg4,arg5)             ErrorMsgIf5(1,fmt,arg1,arg2,arg3,arg4,arg5)

#define WarnMsgIf(expr,fmt)                                 do { if (expr) { techlog.Print(__FILE__,__LINE__,kWarning,(fmt)); } } while(0)
#define WarnMsgIf1(expr,fmt,arg1)                           do { if (expr) { techlog.Print(__FILE__,__LINE__,kWarning,(fmt),(arg1)); } } while(0)
#define WarnMsgIf2(expr,fmt,arg1,arg2)                      do { if (expr) { techlog.Print(__FILE__,__LINE__,kWarning,(fmt),(arg1),(arg2)); } } while(0)
#define WarnMsgIf3(expr,fmt,arg1,arg2,arg3)                 do { if (expr) { techlog.Print(__FILE__,__LINE__,kWarning,(fmt),(arg1),(arg2),(arg3)); } } while(0)
#define WarnMsgIf4(expr,fmt,arg1,arg2,arg3,arg4)            do { if (expr) { techlog.Print(__FILE__,__LINE__,kWarning,(fmt),(arg1),(arg2),(arg3),(arg4)); } } while(0)
#define WarnMsgIf5(expr,fmt,arg1,arg2,arg3,arg4,arg5)       do { if (expr) { techlog.Print(__FILE__,__LINE__,kWarning,(fmt),(arg1),(arg2),(arg3),(arg4),(arg5)); } } while(0)

#define WarnMsg(fmt)                                        WarnMsgIf(1,fmt)
#define WarnMsg1(fmt,arg1)                                  WarnMsgIf1(1,fmt,arg1)
#define WarnMsg2(fmt,arg1,arg2)                             WarnMsgIf2(1,fmt,arg1,arg2)
#define WarnMsg3(fmt,arg1,arg2,arg3)                        WarnMsgIf3(1,fmt,arg1,arg2,arg3)
#define WarnMsg4(fmt,arg1,arg2,arg3,arg4)                   WarnMsgIf4(1,fmt,arg1,arg2,arg3,arg4)
#define WarnMsg5(fmt,arg1,arg2,arg3,arg4,arg5)              WarnMsgIf5(1,fmt,arg1,arg2,arg3,arg4,arg5)

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHLOG_H
