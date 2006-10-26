///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/thread.h"
#include "tech/techtime.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cmath>
#include <cfloat>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sched.h>
#include <unistd.h>
#endif

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

void ThreadSleep(uint milliseconds)
{
#ifdef _WIN32
   Sleep(milliseconds);
#else
   uint secs = milliseconds / 1000;
   if (milliseconds > 0 && secs == 0)
   {
      secs = 1;
   }
   sleep(secs);
#endif
}

///////////////////////////////////////////////////////////////////////////////

tThreadId ThreadGetCurrentId()
{
#ifdef _WIN32
   return GetCurrentThreadId();
#else
   return pthread_self();
#endif
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
struct sThreadName
{
   DWORD dwType; // must be 0x1000
   LPCSTR szName; // pointer to name (in user addr space)
   DWORD dwThreadId; // thread ID (-1 means the calling thread)
   DWORD dwFlags; // must be zero
};
#endif

void ThreadSetName(tThreadId threadId, const char * pszName)
{
#ifdef _WIN32
   sThreadName threadName;
   threadName.dwType = 0x1000;
   threadName.szName = pszName;
   threadName.dwThreadId = threadId;
   threadName.dwFlags = 0;

   __try
   {
      RaiseException(0x406D1388, 0, sizeof(threadName) / sizeof(DWORD), reinterpret_cast<DWORD *>(&threadName));
   }
   __except (EXCEPTION_CONTINUE_EXECUTION)
   {
   }
#endif
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThread
//

////////////////////////////////////////

cThread::cThread()
 : m_threadId(0)
#ifdef _WIN32
 , m_hThread(NULL)
#else
 , m_bJoined(false)
#endif
{
}

////////////////////////////////////////

cThread::~cThread()
{
}

////////////////////////////////////////

bool cThread::Create(int priority, uint stackSize)
{
#ifdef _WIN32
   if (m_hThread == NULL)
   {
      Assert(m_threadId == 0);
      uint threadId = 0;
      m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, stackSize, ThreadEntry, this, CREATE_SUSPENDED, &threadId));
      if (m_hThread != NULL)
      {
         m_threadId = threadId;
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

////////////////////////////////////////

void cThread::Join()
{
#ifdef _WIN32
   WaitForSingleObject(m_hThread, INFINITE);
#else
   m_bJoined = true;
   pthread_join(m_thread, NULL);
#endif
}

////////////////////////////////////////

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

////////////////////////////////////////

#ifdef _WIN32
uint STDCALL cThread::ThreadEntry(void * param)
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

   // If pthread_join is never called for a joinable thread,
   // pthread_detach should be called
   if (!m_bJoined)
   {
      pthread_detach(pThread->m_thread);
   }
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadEvent
//

////////////////////////////////////////

cThreadEvent::cThreadEvent()
#ifdef _WIN32
 : m_hEvent(NULL)
#endif
{
}

////////////////////////////////////////

cThreadEvent::~cThreadEvent()
{
#ifdef _WIN32
   if (m_hEvent != NULL)
   {
      CloseHandle(m_hEvent);
      m_hEvent = NULL;
   }
#endif
}

////////////////////////////////////////

bool cThreadEvent::Create()
{
#ifdef _WIN32
   if (m_hEvent != NULL)
   {
      return false;
   }

   m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

   return (m_hEvent != NULL);
#else
#pragma message("Need platform-specific event create function")
#endif
}

////////////////////////////////////////

bool cThreadEvent::Wait(uint timeout)
{
#ifdef _WIN32
   if (m_hEvent != NULL)
   {
      if (WaitForSingleObject(m_hEvent, timeout) == WAIT_OBJECT_0)
      {
         return true;
      }
   }
#else
#pragma message("Need platform-specific event wait function")
#endif
   return false;
}

////////////////////////////////////////

bool cThreadEvent::Set()
{
#ifdef _WIN32
   if ((m_hEvent != NULL) && SetEvent(m_hEvent))
   {
      return true;
   }
#else
#pragma message("Need platform-specific event reset function")
#endif
   return false;
}

////////////////////////////////////////

bool cThreadEvent::Reset()
{
#ifdef _WIN32
   if ((m_hEvent != NULL) && ResetEvent(m_hEvent))
   {
      return true;
   }
#else
#pragma message("Need platform-specific event reset function")
#endif
   return false;
}

////////////////////////////////////////

bool cThreadEvent::Pulse()
{
#ifdef _WIN32
   if ((m_hEvent != NULL) && PulseEvent(m_hEvent))
   {
      return true;
   }
#else
#pragma message("Need platform-specific event pulse function")
#endif
   return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadMutex
//

////////////////////////////////////////

cThreadMutex::cThreadMutex()
#ifdef _WIN32
 : m_hMutex(NULL)
#endif
{
}

////////////////////////////////////////

cThreadMutex::~cThreadMutex()
{
#ifdef _WIN32
   if (m_hMutex != NULL)
   {
      CloseHandle(m_hMutex);
      m_hMutex = NULL;
   }
#else
   pthread_mutex_destroy(&m_mutex);
#endif
}

////////////////////////////////////////

bool cThreadMutex::Create()
{
#ifdef _WIN32
   if (m_hMutex == NULL)
   {
      m_hMutex = CreateMutex(NULL, FALSE, NULL);
      if (m_hMutex != NULL)
      {
         return true;
      }
   }
   return false;
#else
   pthread_mutex_init(&m_mutex, NULL);
   // TODO: test return value
   return true;
#endif
}

////////////////////////////////////////

bool cThreadMutex::Acquire(uint timeout)
{
#ifdef _WIN32
   if (m_hMutex != NULL)
   {
      if (WaitForSingleObject(m_hMutex, timeout) == WAIT_OBJECT_0)
      {
         return true;
      }
   }
   return false;
#else
   pthread_mutex_lock(&m_mutex);
   // TODO: test return value
   return true;
#endif
}

////////////////////////////////////////

bool cThreadMutex::Release()
{
#ifdef _WIN32
   if (m_hMutex != NULL)
   {
      if (ReleaseMutex(m_hMutex))
      {
         return true;
      }
   }
   return false;
#else
   pthread_mutex_unlock(&m_mutex);
   // TODO: test return value
   return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMutexLock
//

////////////////////////////////////////

cMutexLock::cMutexLock(cThreadMutex * pMutex)
 : m_pMutex(pMutex),
   m_bLocked(false)
{
   Assert(pMutex != NULL);
}

////////////////////////////////////////

cMutexLock::~cMutexLock()
{
   Release();
}

////////////////////////////////////////

bool cMutexLock::Acquire(uint timeout)
{
   if (!m_bLocked && m_pMutex != NULL)
   {
      m_bLocked = m_pMutex->Acquire(timeout);
   }
   return m_bLocked;
}

////////////////////////////////////////

void cMutexLock::Release()
{
   if (m_bLocked && m_pMutex != NULL)
   {
      if (m_pMutex->Release())
      {
         m_bLocked = false;
      }
   }
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

TEST(ThreadSleep)
{
   cThreadEvent event;
   CHECK(event.Create());

   static const double kWaitSecs = 2;
   static const ulong kWaitMillis = (ulong)(kWaitSecs * 1000);

   class cSleepThread : public cThread
   {
   public:
      cSleepThread(cThreadEvent * pEvent, ulong sleepMs)
        : m_pEvent(pEvent), m_sleepMs(sleepMs)
      {
      }

      virtual int Run()
      {
         ThreadSleep(m_sleepMs);
         m_pEvent->Set();
         return 0;
      }

   private:
      cThreadEvent * m_pEvent;
      ulong m_sleepMs;
   };

   cSleepThread * pThread = new cSleepThread(&event, kWaitMillis);

   double elapsed = -TimeGetSecs();

   CHECK(pThread->Create());
   CHECK(event.Wait());

   elapsed += TimeGetSecs();

   delete pThread, pThread = NULL;

   CHECK(elapsed > (kWaitSecs - 1.0e-2));
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
