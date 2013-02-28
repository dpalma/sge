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
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#endif

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Thread);
#define LocalMsg(msg)                  DebugMsgEx(Thread,(msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(Thread,(msg),(a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(Thread,(msg),(a1),(a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(Thread,(cond),(msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(Thread,(cond),(msg),(a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(Thread,(cond),(msg),(a1),(a2))


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
   LocalMsg2("Thread priority %d mapped to %d\n", priority, schedParam.sched_priority);
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
   if (!pThread->m_bJoined)
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
#else
 : m_bInitialized(false)
#endif
{
}

////////////////////////////////////////

cThreadEvent::~cThreadEvent()
{
   Destroy();
}

////////////////////////////////////////

bool cThreadEvent::Create()
{
#ifdef _WIN32
   if (m_hEvent != NULL)
   {
      return false;
   }

   // Create an auto-reset event because it's behavior matches
   // pthreads condition variables more closely.
   m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

   return (m_hEvent != NULL);
#else
   if (m_bInitialized)
   {
      return false;
   }
   pthread_mutex_init(&m_mutex, NULL);
   pthread_cond_init(&m_cond, NULL);
   m_bInitialized = true;
   return true;
#endif
}

////////////////////////////////////////

void cThreadEvent::Destroy()
{
#ifdef _WIN32
   if (m_hEvent != NULL)
   {
      CloseHandle(m_hEvent);
      m_hEvent = NULL;
   }
#else
   if (m_bInitialized)
   {
      pthread_cond_destroy(&m_cond);
      pthread_mutex_destroy(&m_mutex);
      m_bInitialized = false;
   }
#endif
}

////////////////////////////////////////

bool cThreadEvent::Wait()
{
   bool bResult = false;
#ifdef _WIN32
   if (m_hEvent != NULL)
   {
      if (WaitForSingleObject(m_hEvent, INFINITE) == WAIT_OBJECT_0)
      {
         bResult = true;
      }
   }
#else
   if (m_bInitialized)
   {
      pthread_mutex_lock(&m_mutex);
      if (pthread_cond_wait(&m_cond, &m_mutex) == 0)
      {
         bResult = true;
      }
      pthread_mutex_unlock(&m_mutex);
   }
#endif
   return bResult;
}

////////////////////////////////////////

#ifdef __GNUC__
static struct timespec * MillisecsFromNow(uint millisecs, struct timespec * ts)
{
   struct timeb curTime;
   int64 nanosecs, secs;
   const int64 kNanosecsPerMillisec = 1000000;
   const int64 kNanosecsPerSec = 1000000000;
   ftime(&curTime);
   nanosecs = ((int64)(millisecs + curTime.millitm)) * kNanosecsPerMillisec;
   if (nanosecs >= kNanosecsPerSec)
   {
      secs = curTime.time + 1;
      nanosecs %= kNanosecsPerSec;
   }
   else
   {
      secs = curTime.time;
   }

   ts->tv_nsec = nanosecs;
   ts->tv_sec = secs;
   return ts;
}
#endif

bool cThreadEvent::Wait(uint timeout)
{
   bool bResult = false;
#ifdef _WIN32
   if (m_hEvent != NULL)
   {
      if (WaitForSingleObject(m_hEvent, timeout) == WAIT_OBJECT_0)
      {
         bResult = true;
      }
   }
#else
   if (m_bInitialized)
   {
      struct timespec timeoutSpec;
      pthread_mutex_lock(&m_mutex);
      if (pthread_cond_timedwait(&m_cond, &m_mutex, MillisecsFromNow(timeout, &timeoutSpec)) == 0)
      {
         bResult = true;
      }
      pthread_mutex_unlock(&m_mutex);
   }
#endif
   return bResult;
}

////////////////////////////////////////

bool cThreadEvent::Signal()
{
#ifdef _WIN32
   // Release a single thread waiting on the event.  See
   // documentation of SetEvent for auto-reset events.
   if ((m_hEvent != NULL) && SetEvent(m_hEvent))
   {
      return true;
   }
#else
   if (m_bInitialized)
   {
      if (pthread_cond_signal(&m_cond) == 0)
      {
         return true;
      }
   }
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
 : m_pMutex(pMutex)
 , m_bLocked(false)
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
         m_pEvent->Signal();
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
