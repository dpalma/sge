///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifdef _DEBUG // (entire file)

#include <climits>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <locale>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOGDI
#include <windows.h>
#include <crtdbg.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG

struct sNamedLogChannel
{
   const char * pszName;
   bool * pbEnabled;
   struct sNamedLogChannel * pNext;
};

sNamedLogChannel * g_pNamedLogChannels = NULL;

struct sNamedLogChannelsAutoCleanup
{
   ~sNamedLogChannelsAutoCleanup()
   {
      sNamedLogChannel * p = g_pNamedLogChannels;
      while (p != NULL)
      {
         g_pNamedLogChannels = g_pNamedLogChannels->pNext;
         delete p;
         p = g_pNamedLogChannels;
      }
   }
} g_namedLogChannelsAutoCleanup;

///////////////////////////////////////

static bool * LogFindChannel(const char * pszName)
{
   Assert(pszName != NULL && *pszName != 0);

   sNamedLogChannel * p = g_pNamedLogChannels;
   for (; p != NULL; p = p->pNext)
   {
      if (strcmp(p->pszName, pszName) == 0)
         return p->pbEnabled;
   }

   return NULL;
}

///////////////////////////////////////

bool LogDefineChannel(const char * pszName, bool * pbEnabled)
{
   Assert(pszName != NULL && *pszName != 0);
   Assert(pbEnabled != NULL);

   bool * pb = LogFindChannel(pszName);
   if (pb == NULL)
   {
      sNamedLogChannel * p = new sNamedLogChannel;
      p->pszName = pszName;
      p->pbEnabled = pbEnabled;
      p->pNext = g_pNamedLogChannels;
      g_pNamedLogChannels = p;
   }
   else
   {
      // Should not have two separate bools for the same log channel
      Assert(pb == pbEnabled);
   }

   return *pbEnabled;
}

///////////////////////////////////////

void LogEnableChannel(const char * pszName, bool bEnabled)
{
   Assert(pszName != NULL && *pszName != 0);

   bool * pb = LogFindChannel(pszName);
   if (pb != NULL)
   {
      *pb = bEnabled;
   }

   DebugMsgIf1(!pb, "Log channel \"%s\" not found\n", pszName);
}

#endif // !NDEBUG

///////////////////////////////////////////////////////////////////////////////

FILE * g_pDebugEchoFile = NULL;

struct sDebugEchoFileAutoClose
{
   ~sDebugEchoFileAutoClose()
   {
      DebugEchoFileStop();
   }
} g_debugEchoFileAutoClose;

///////////////////////////////////////////////////////////////////////////////
// Shows the user an assertion failure dialog box. Returns true if
// the user wants to break into the debugger.

bool AssertFail(const char * pszFile, int line, const char * pszExpr)
{
#ifdef _MSC_VER
   // remove WM_QUIT because if it's in the queue then the message
   // box won't display
   MSG msg;
   BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
   bool bResult = _CrtDbgReport(_CRT_ASSERT, pszFile, line, NULL, pszExpr)
      ? true : false;
   if (bQuit)
      PostQuitMessage(msg.wParam);
#else
   bool bResult = false;
   DebugPrintf(pszFile, line, "ASSERTION FAILURE: %s\n", pszExpr);
#endif
   return bResult;
}

///////////////////////////////////////////////////////////////////////////////

void DebugPrintf(const char * pszFile, int line, const char * pszFormat, ...)
{
   static char szBuffer[0x400];

   int nWritten = 0;
   if (pszFile && line)
   {
      nWritten = snprintf(szBuffer, sizeof(szBuffer),
                          "%s(%d) : ", pszFile, line);
   }

   va_list args;
   va_start(args, pszFormat);
   vsnprintf(szBuffer + nWritten, sizeof(szBuffer) - nWritten,
             pszFormat, args);
   va_end(args);

#ifdef _MSC_VER
   OutputDebugStringA(szBuffer);
#endif
   fputs(szBuffer, stderr);
   if (g_pDebugEchoFile != NULL)
      fputs(szBuffer, g_pDebugEchoFile);
}

///////////////////////////////////////////////////////////////////////////////

static void TimeAsString(char * psz, int max)
{
   time_t t;
   time(&t);

   strncpy(psz, asctime(localtime(&t)), max);
   psz[max - 1] = 0;

   char * pEnd = psz + strlen(psz) - 1;
   while (pEnd >= psz && isspace(*pEnd))
      *pEnd-- = 0;
}

///////////////////////////////////////////////////////////////////////////////

void DebugEchoFileStart(const char * pszFile)
{
   if (g_pDebugEchoFile == NULL)
   {
      g_pDebugEchoFile = fopen(pszFile, "w");

      if (g_pDebugEchoFile != NULL)
      {
         char szTime[30];
         TimeAsString(szTime, _countof(szTime));
         fprintf(g_pDebugEchoFile, "Log file opened at %s\n", szTime);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

void DebugEchoFileStop()
{
   if (g_pDebugEchoFile != NULL)
   {
      char szTime[30];
      TimeAsString(szTime, _countof(szTime));
      fprintf(g_pDebugEchoFile, "Log file closed at %s\n", szTime);
      fclose(g_pDebugEchoFile);
      g_pDebugEchoFile = NULL;
   }
}

///////////////////////////////////////////////////////////////////////////////

struct sDebugState
{
   sDebugState()
   {
#ifdef _MSC_VER
      // Use the debug allocator and dump all memory leaks at exit
      int crtDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
      _CrtSetDbgFlag(crtDbgFlag | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
   }
} g_debugState;

///////////////////////////////////////////////////////////////////////////////

#endif // _DEBUG (entire file)
