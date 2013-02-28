///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/filespec.h"
#include "tech/readwriteapi.h"

#include <cstring>
#include <cstdarg>

#include "tech/dbgalloc.h" // must be last header

#ifdef _WIN32
extern "C" uint STDCALL OutputDebugStringA(const char *);
extern "C" uint STDCALL OutputDebugStringW(const wchar_t *);
#define vsnprintf _vsnprintf
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLog
//

///////////////////////////////////////

cLog techlog;

///////////////////////////////////////

cLog::cLogChannel::cLogChannel(const tChar * pszName, bool * pbEnabled, cLogChannel * pNext)
 : m_pszName(pszName)
 , m_pbEnabled(pbEnabled)
 , m_pNext(pNext)
{
}

///////////////////////////////////////

cLog::cLog()
{
   // No initialization is done in the constructor for a reason.
   // DefineChannel() may be called at static initialization time,
   // possibly before the constructor for techlog is called.
   // Initializing member variables here would mess up work done
   // by DefineChannel() in this case. Instances of cLogInitializer
   // will call Initialize(). All initialization should be done there
   // instead.
}

///////////////////////////////////////

cLog::~cLog()
{
}

///////////////////////////////////////

void cLog::Initialize()
{
   m_pChannels = NULL;
   m_callback = NULL;
   m_pLogFile = NULL;
}

///////////////////////////////////////

void cLog::Shutdown()
{
   cLogChannel * p = m_pChannels;
   while (p != NULL)
   {
      m_pChannels = m_pChannels->GetNext();
      delete p;
      p = m_pChannels;
   }

   CloseLogFile();
}

///////////////////////////////////////

bool cLog::OpenLogFile(const cFileSpec & logFile)
{
   if (m_pLogFile == NULL)
   {
      cAutoIPtr<IWriter> pWriter; 
      if (FileWriterCreate(logFile, kFileModeText, &pWriter) == S_OK)
      {
         m_pLogFile = CTAddRef(pWriter);
         return true;
      }
   }
   return false;
}

///////////////////////////////////////

void cLog::CloseLogFile()
{
   if (m_pLogFile != NULL)
   {
      reinterpret_cast<IWriter *>(m_pLogFile)->Release();
      m_pLogFile = NULL;
   }
}

///////////////////////////////////////

bool cLog::DefineChannel(const tChar * pszChannel, bool * pEnableFlag)
{
   Assert(pszChannel != NULL && *pszChannel != 0);
   Assert(pEnableFlag != NULL);

   bool bResult = false;

   bool * pb = FindChannel(pszChannel);
   if (pb == NULL)
   {
#ifdef _MSC_VER
      int crtDbgFlag = _CrtSetDbgFlag(0);
#endif

      cLogChannel * pLogChannel = new cLogChannel(pszChannel, pEnableFlag, m_pChannels);
      if (pLogChannel != NULL)
      {
         m_pChannels = pLogChannel;
         bResult = true;
      }

#ifdef _MSC_VER
      _CrtSetDbgFlag(crtDbgFlag);
#endif
   }
   else
   {
      // Should not have two separate bools for the same log channel
      Assert(pb == pEnableFlag);
   }

   return bResult;
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

void cLog::Print(const tChar * pszFile, int line, eLogSeverity severity, const tChar * pszFormat, ...)
{
   size_t fileAndLineLen = 0; // length of the file and line number part of the string
   if (pszFile && line)
   {
#if _MSC_VER >= 1400
      fileAndLineLen = _sntprintf_s(m_szBuffer, sizeof(m_szBuffer), _countof(m_szBuffer), _T("%s(%d) : "), pszFile, line);
#else
      fileAndLineLen = _sntprintf(m_szBuffer, _countof(m_szBuffer), _T("%s(%d) : "), pszFile, line);
#endif
   }

   tChar * pMsgBuffer = m_szBuffer + fileAndLineLen;
   size_t sizeofMsgBuffer = sizeof(m_szBuffer) - (fileAndLineLen * sizeof(tChar));
   size_t maxMsgBufferChars = _countof(m_szBuffer) - fileAndLineLen;

   va_list args;
   va_start(args, pszFormat);
#if _MSC_VER >= 1400
   _vsntprintf_s(pMsgBuffer, sizeofMsgBuffer, maxMsgBufferChars, pszFormat, args);
#else
   _vsntprintf(pMsgBuffer, maxMsgBufferChars, pszFormat, args);
#endif
   va_end(args);

   if (m_pLogFile != NULL)
   {
      IWriter * pLogWriter = reinterpret_cast<IWriter *>(m_pLogFile);
      pLogWriter->Write(pMsgBuffer);
   }

   if (m_callback != NULL)
   {
      (*m_callback)(severity, m_szBuffer, _tcslen(m_szBuffer));
   }

#ifdef _WIN32
#ifdef _UNICODE
   OutputDebugStringW(m_szBuffer);
#else
   OutputDebugStringA(m_szBuffer);
#endif
#endif

#ifdef __GNUC__
   printf(m_szBuffer);
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

bool * cLog::FindChannel(const tChar * pszChannel) const
{
   Assert(pszChannel != NULL && *pszChannel != 0);

   cLogChannel * pLogChannel = m_pChannels;
   while (pLogChannel != NULL)
   {
      if (_tcscmp(pLogChannel->GetName(), pszChannel) == 0)
      {
         return pLogChannel->GetEnabledPointer();
      }
      pLogChannel = pLogChannel->GetNext();
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogInitializer
//

///////////////////////////////////////

long cLogInitializer::gm_initCount = 0;

///////////////////////////////////////

cLogInitializer::cLogInitializer()
{
   if (gm_initCount == 0)
   {
      techlog.Initialize();
   }
   gm_initCount++;
}

///////////////////////////////////////

cLogInitializer::~cLogInitializer()
{
   gm_initCount--;
   if (gm_initCount == 0)
   {
      techlog.Shutdown();
   }
}

///////////////////////////////////////////////////////////////////////////////
