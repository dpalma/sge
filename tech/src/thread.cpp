///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "thread.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThread
//

///////////////////////////////////////

cThread::cThread()
#ifdef _WIN32
 : m_hThread(NULL)
#else
#error ("Need platform-specific thread member initialization")
#endif
 , m_threadId(0)
{
}

///////////////////////////////////////

cThread::~cThread()
{
}

///////////////////////////////////////

bool cThread::Begin()
{
   if (m_hThread == NULL)
   {
      m_hThread = (HTHREAD)CreateThread(NULL, 0, ThreadEntry, this, 0, &m_threadId);
      return (m_hThread != NULL);
   }
   return false;
}

///////////////////////////////////////

uint cThread::End()
{
   // TODO
   return 0;
}

///////////////////////////////////////

void cThread::Join(uint timeoutMs)
{
   WaitForSingleObject(GetHandle(), timeoutMs);
}

///////////////////////////////////////

bool cThread::Terminate()
{
   if (m_hThread != NULL)
   {
      return TerminateThread(m_hThread, 0) ? true : false;
   }
   return false;
}

///////////////////////////////////////

ulong STDCALL cThread::ThreadEntry(void * param)
{
   cThread * pThread = (cThread *)param;
   Assert(pThread != NULL);

   int result = pThread->Run();

   return result;
}

///////////////////////////////////////////////////////////////////////////////
