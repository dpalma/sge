///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "techlog.h"

#include <cstdarg>

#include "dbgalloc.h" // must be last header

#ifdef _WIN32
extern "C" uint STDCALL OutputDebugStringA(const char *);
extern "C" uint STDCALL OutputDebugStringW(const tChar *);
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLog
//

///////////////////////////////////////

TECH_API cLog techlog;

///////////////////////////////////////

tChar cLog::gm_szBuffer[0x400];

///////////////////////////////////////

cLog::cLog()
 : m_pChannels(NULL),
   m_callback(NULL)
{
}

///////////////////////////////////////

cLog::~cLog()
{
   sChannel * p = m_pChannels;
   while (p != NULL)
   {
      m_pChannels = m_pChannels->pNext;
      delete p;
      p = m_pChannels;
   }
}

///////////////////////////////////////

bool cLog::DefineChannel(const tChar * pszChannel, bool * pEnableFlag)
{
   Assert(pszChannel != NULL && *pszChannel != 0);
   Assert(pEnableFlag != NULL);

   bool * pb = FindChannel(pszChannel);
   if (pb == NULL)
   {
#ifdef _MSC_VER
      int crtDbgFlag = _CrtSetDbgFlag(0);
#endif
      sChannel * p = new sChannel;
      p->pszName = pszChannel;
      p->pbEnabled = pEnableFlag;
      p->pNext = m_pChannels;
      m_pChannels = p;
#ifdef _MSC_VER
      _CrtSetDbgFlag(crtDbgFlag);
#endif
   }
   else
   {
      // Should not have two separate bools for the same log channel
      Assert(pb == pEnableFlag);
   }

   return *pEnableFlag;
}

///////////////////////////////////////

bool cLog::EnableChannel(const tChar * pszChannel, bool bEnable)
{
   Assert(pszChannel != NULL && *pszChannel != 0);

   bool * pb = FindChannel(pszChannel);
   if (pb != NULL)
   {
      *pb = bEnable;
      return true;
   }

   DebugMsgIf1(!pb, "Log channel \"%s\" not found\n", pszChannel);
   return false;
}

///////////////////////////////////////

void cLog::Print(eLogSeverity severity, const tChar * pszFormat, ...)
{
   va_list args;
   va_start(args, pszFormat);
   vsnprintf(gm_szBuffer, _countof(gm_szBuffer), pszFormat, args);
   va_end(args);

   if (m_callback != NULL)
   {
      (*m_callback)(severity, gm_szBuffer, strlen(gm_szBuffer));
   }

#ifdef _WIN32
#ifdef _UNICODE
   OutputDebugStringW(gm_szBuffer);
#else
   OutputDebugStringA(gm_szBuffer);
#endif
#endif
}

///////////////////////////////////////

void cLog::Print(const char * pszFile, int line, eLogSeverity severity, const tChar * pszFormat, ...)
{
   size_t len = 0;
   if (pszFile && line)
   {
      len = snprintf(gm_szBuffer, _countof(gm_szBuffer), "%s(%d) : ", pszFile, line);
   }

   va_list args;
   va_start(args, pszFormat);
   vsnprintf(gm_szBuffer + len, _countof(gm_szBuffer) - len, pszFormat, args);
   va_end(args);

   if (m_callback != NULL)
   {
      (*m_callback)(severity, gm_szBuffer, strlen(gm_szBuffer));
   }

#ifdef _WIN32
#ifdef _UNICODE
   OutputDebugStringW(gm_szBuffer);
#else
   OutputDebugStringA(gm_szBuffer);
#endif
#endif
}

///////////////////////////////////////

tLogCallbackFn cLog::SetCallback(tLogCallbackFn pfn)
{
   tLogCallbackFn pfnFormer = m_callback;
   m_callback = pfn;
   return pfnFormer;
}

///////////////////////////////////////

bool * cLog::FindChannel(const tChar * pszChannel)
{
   Assert(pszChannel != NULL && *pszChannel != 0);

   sChannel * p = m_pChannels;
   for (; p != NULL; p = p->pNext)
   {
      if (strcmp(p->pszName, pszChannel) == 0)
      {
         return p->pbEnabled;
      }
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
