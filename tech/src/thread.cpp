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

#ifdef HAVE_CPPUNIT
#include "techtime.h"
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

void ThreadSleep(uint milliseconds)
{
#ifdef _WIN32
   Sleep(milliseconds);
#else
#error ("Need platform-specific thread sleep function")
#endif
}

uint ThreadGetCurrentId()
{
#ifdef _WIN32
   return GetCurrentThreadId();
#else
#error ("Need platform-specific get current thread ID function")
   return 0;
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
#ifdef _WIN32
 : m_hThread(NULL)
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

////////////////////////////////////////

void cThread::Join()
{
#ifdef _WIN32
   WaitForSingleObject(m_hThread, INFINITE);
#else
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
//
// CLASS: cThreadEvent
//

////////////////////////////////////////

cThreadEvent::cThreadEvent()
 : m_hEvent(NULL)
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
#error ("Need platform-specific event create function")
#endif
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
#error ("Need platform-specific event reset function")
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
#error ("Need platform-specific event reset function")
#endif
   return false;
}

////////////////////////////////////////

void cThreadEvent::Wait(uint timeout)
{
#ifdef _WIN32
   if (m_hEvent != NULL)
   {
      WaitForSingleObject(m_hEvent, timeout);
   }
#else
#error ("Need platform-specific event wait function")
#endif
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cThreadTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cThreadTests);
      CPPUNIT_TEST(TestSleep);
   CPPUNIT_TEST_SUITE_END();

   void TestSleep();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cThreadTests);

////////////////////////////////////////

void cThreadTests::TestSleep()
{
   cThreadEvent event;

   CPPUNIT_ASSERT(event.Create());

#define kWaitMillis 2000

   class cSleepThread : public cThread
   {
   public:
      cSleepThread(cThreadEvent * pEvent)
        : m_pEvent(pEvent)
      {
         CPPUNIT_ASSERT(pEvent != NULL);
      }

      virtual int Run()
      {
         ThreadSleep(kWaitMillis);
         CPPUNIT_ASSERT(m_pEvent->Set());
         return 0;
      }

   private:
      cThreadEvent * m_pEvent;
   };

   double start = TimeGetSecs();

   cSleepThread * pThread = new cSleepThread(&event);
   CPPUNIT_ASSERT(pThread->Create());

   event.Wait();

   double end = TimeGetSecs();

   delete pThread, pThread = NULL;

   CPPUNIT_ASSERT(((end - start) * 1000) >= kWaitMillis);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
