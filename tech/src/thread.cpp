///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "thread.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/types.h>
#include <pthread.h>
#include <sched.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThread
//

///////////////////////////////////////

cThread::cThread()
#ifdef _WIN32
 : m_hThread(NULL)
#endif
{
}

///////////////////////////////////////

cThread::~cThread()
{
}

///////////////////////////////////////

static int MapThreadPriority(int priority)
{
   Assert(priority >= kTP_Lowest && priority <= kTP_Highest);
#ifdef _WIN32
   if (priority == kTP_Lowest)
   {
      return THREAD_PRIORITY_LOWEST;
   }
   else if (priority == kTP_Highest)
   {
      return THREAD_PRIORITY_HIGHEST;
   }
   else if (priority == kTP_Normal)
   {
      return THREAD_PRIORITY_NORMAL;
   }
   else
   {
      return THREAD_PRIORITY_LOWEST + ((priority - kTP_Lowest) * (THREAD_PRIORITY_HIGHEST - THREAD_PRIORITY_LOWEST) / (kTP_Highest - kTP_Lowest));
   }
#else
   if (priority == kTP_Lowest)
   {
      return 1;
   }
   else if (priority == kTP_Highest)
   {
      return 127;
   }
   else if (priority == kTP_Normal)
   {
      return 64;
   }
   else
   {
      return 1 + ((priority - kTP_Lowest) * 126 / (kTP_Highest - kTP_Lowest));
   }
#endif
}

bool cThread::Create(int priority, uint stackSize)
{
#ifdef _WIN32
   if (m_hThread == NULL)
   {
      ulong threadId;
      m_hThread = CreateThread(NULL, 0, ThreadEntry, this, CREATE_SUSPENDED, &threadId);
      if (m_hThread != NULL)
      {
         SetThreadPriority(m_hThread, MapThreadPriority(priority));
         ResumeThread(m_hThread);
         return true;
      }
   }
   return false;
#else
   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   if (stackSize > 0)
   {
      pthread_attr_setstacksize(&attr, stackSize);
   }
   struct sched_param schedParam;
   schedParam.sched_priority = MapThreadPriority(priority);
   DebugMsg2("Thread priority %d mapped to %d\n", priority, schedParam.sched_priority);
   pthread_attr_setschedparam(&attr, &schedParam);
   int result = pthread_create(&m_thread, &attr, ThreadEntry, this); 
   pthread_attr_destroy(&attr);
   return (result == 0);
#endif
}

///////////////////////////////////////

void cThread::Join()
{
#ifdef _WIN32
   WaitForSingleObject(m_hThread, INFINITE);
#else
   pthread_join(m_thread, NULL);
#endif
}

///////////////////////////////////////

bool cThread::Terminate()
{
#ifdef _WIN32
   if (m_hThread != NULL)
   {
      return TerminateThread(m_hThread, 0) ? true : false;
   }
   else
   {
      return false;
   }
#else
   return (pthread_kill(m_thread, SIGKILL) == 0);
#endif
}

///////////////////////////////////////

#ifdef _WIN32
ulong STDCALL cThread::ThreadEntry(void * param)
{
   cThread * pThread = (cThread *)param;
   Assert(pThread != NULL);

   int result = pThread->Run();

   return result;
}
#else
void * cThread::ThreadEntry(void * param)
{
   cThread * pThread = (cThread *)param;
   Assert(pThread != NULL);

   int result = pThread->Run();
}
#endif

///////////////////////////////////////////////////////////////////////////////
