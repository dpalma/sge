////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "threadcaller.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(ThreadCaller);

#define LocalMsg(msg)            DebugMsgEx(ThreadCaller,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(ThreadCaller,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ThreadCaller,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ThreadCaller,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ThreadCaller,(msg),(a),(b),(c),(d))

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadCaller
//

////////////////////////////////////////

cThreadCaller::cThreadCaller()
 : cGlobalObject<IMPLEMENTS(IThreadCaller)>("ThreadCaller")
{
}

////////////////////////////////////////

cThreadCaller::~cThreadCaller()
{
}

////////////////////////////////////////

tResult cThreadCaller::Init()
{
   if (!m_mutex.Create())
   {
      ErrorMsg("Error creating mutex\n");
      return E_FAIL;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cThreadCaller::Term()
{
   cMutexLock lock(&m_mutex);
   if (lock.Acquire())
   {
      tThreadCalls::iterator iterCalls = m_calls.begin();
      tThreadCalls::iterator endCalls = m_calls.end();
      for (; iterCalls != endCalls; iterCalls++)
      {
         delete iterCalls->second;
      }

      m_calls.clear();

      tReceiptEventMap::iterator iterReceiptEvents = m_receiptEvents.begin();
      tReceiptEventMap::iterator endReceiptEvents = m_receiptEvents.end();
      for (; iterReceiptEvents != endReceiptEvents; iterReceiptEvents++)
      {
         iterReceiptEvents->second->Pulse();
         delete iterReceiptEvents->second;
      }

      m_receiptEvents.clear();
   }

   return S_OK;
}

////////////////////////////////////////

tResult cThreadCaller::ThreadInit()
{
   cMutexLock lock(&m_mutex);
   if (lock.Acquire())
   {
      uint threadId = ThreadGetCurrentId();

      if (m_receiptEvents.find(threadId) != m_receiptEvents.end())
      {
         // Thread already initialized
         return S_FALSE;
      }

      cThreadEvent * pEvent = new cThreadEvent;
      if (pEvent == NULL)
      {
         return E_OUTOFMEMORY;
      }

      if (!pEvent->Create())
      {
         delete pEvent;
         return E_FAIL;
      }

      m_receiptEvents.insert(std::make_pair(threadId, pEvent));

      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cThreadCaller::ThreadTerm()
{
   cMutexLock lock(&m_mutex);
   if (lock.Acquire())
   {
      uint threadId = ThreadGetCurrentId();

      m_calls.erase(threadId);

      tReceiptEventMap::iterator iterReceiptEvent = m_receiptEvents.find(threadId);
      if (iterReceiptEvent != m_receiptEvents.end())
      {
         iterReceiptEvent->second->Pulse();
         delete iterReceiptEvent->second;
         m_receiptEvents.erase(iterReceiptEvent);
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cThreadCaller::ThreadIsInitialized(uint threadId)
{
   cMutexLock lock(&m_mutex);
   if (lock.Acquire())
   {
      tReceiptEventMap::iterator iterReceiptEvent = m_receiptEvents.find(threadId);
      if (iterReceiptEvent != m_receiptEvents.end())
      {
         return S_OK;
      }

      return S_FALSE;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cThreadCaller::ReceiveCalls(uint * pnCalls)
{
   cMutexLock lock(&m_mutex);
   if (lock.Acquire())
   {
      uint threadId = ThreadGetCurrentId();

      tReceiptEventMap::iterator iterReceiptEvent = m_receiptEvents.find(threadId);
      if (iterReceiptEvent == m_receiptEvents.end())
      {
         return E_FAIL;
      }

      uint nCalls = 0;

      tThreadCalls::iterator iter = m_calls.lower_bound(threadId);
      tThreadCalls::iterator end = m_calls.upper_bound(threadId);
      for (; iter != end; iter++)
      {
         (*iter->second)();
         delete iter->second;
         nCalls++;
      }

      if (pnCalls != NULL)
      {
         *pnCalls = nCalls;
      }

      m_calls.erase(threadId);

      iterReceiptEvent->second->Pulse();

      return (nCalls > 0) ? S_OK : S_FALSE;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cThreadCaller::PostCall(uint threadId, cFunctor * pFunctor)
{
   if (pFunctor == NULL)
   {
      return E_POINTER;
   }

   cMutexLock lock(&m_mutex);
   if (lock.Acquire())
   {
      m_calls.insert(std::make_pair(threadId, pFunctor));
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

void ThreadCallerCreate()
{
   cAutoIPtr<IThreadCaller>(new cThreadCaller);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cThreadCallerTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cThreadCallerTests);
      CPPUNIT_TEST(TestPostCall);
   CPPUNIT_TEST_SUITE_END();

   static ulong g_nFooCalls;

   static void Foo();

   void TestPostCall();

public:
   virtual void setUp();
   virtual void tearDown();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cThreadCallerTests);

////////////////////////////////////////

ulong cThreadCallerTests::g_nFooCalls = 0;

////////////////////////////////////////

void cThreadCallerTests::Foo()
{
   g_nFooCalls++;
}

////////////////////////////////////////

void cThreadCallerTests::TestPostCall()
{
   class cSetFooThread : public cThread
   {
   public:
      virtual int Run()
      {
         UseGlobal(ThreadCaller);
         CPPUNIT_ASSERT(SUCCEEDED(pThreadCaller->ThreadInit()));
         for (;;)
         {
            if (pThreadCaller->ReceiveCalls(NULL) == S_OK)
            {
               break;
            }
         }
         CPPUNIT_ASSERT(SUCCEEDED(pThreadCaller->ThreadTerm()));
         return 0;
      }
   };

   cThread * pThread = new cSetFooThread;
   CPPUNIT_ASSERT(pThread->Create());

   UseGlobal(ThreadCaller);

   static const ulong kMaxSleeps = 1000;
   ulong nSleeps = 0;
   while (pThreadCaller->ThreadIsInitialized(pThread->GetThreadId()) != S_OK)
   {
      ThreadSleep(100);
      nSleeps++;
      CPPUNIT_ASSERT(nSleeps < kMaxSleeps);
   }

   CPPUNIT_ASSERT(g_nFooCalls == 0);

   pThreadCaller->PostCall(pThread->GetThreadId(), Foo);
   pThread->Join();

   delete pThread, pThread = NULL;

   CPPUNIT_ASSERT(g_nFooCalls == 1);
}

////////////////////////////////////////

void cThreadCallerTests::setUp()
{
   UseGlobal(ThreadCaller);
   CPPUNIT_ASSERT(SUCCEEDED(pThreadCaller->ThreadInit()));
}

////////////////////////////////////////

void cThreadCallerTests::tearDown()
{
   UseGlobal(ThreadCaller);
   CPPUNIT_ASSERT(SUCCEEDED(pThreadCaller->ThreadTerm()));
}

#endif // HAVE_CPPUNIT

////////////////////////////////////////////////////////////////////////////////
