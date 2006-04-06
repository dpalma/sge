////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "threadcaller.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(ThreadCaller);

#define LocalMsg(msg)            DebugMsgEx(ThreadCaller,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(ThreadCaller,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ThreadCaller,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ThreadCaller,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ThreadCaller,msg,(a),(b),(c),(d))

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadCaller
//

////////////////////////////////////////

cThreadCaller::cThreadCaller()
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
      tThreadId threadId = ThreadGetCurrentId();

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
      tThreadId threadId = ThreadGetCurrentId();

      tThreadCalls::size_type nCallsErased = m_calls.erase(threadId);

      LocalMsg2("Cancelled %d pending calls for thread %d\n", nCallsErased, threadId);

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

tResult cThreadCaller::ThreadIsInitialized(tThreadId threadId)
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
      tThreadId threadId = ThreadGetCurrentId();

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

tResult cThreadCaller::PostCall(tThreadId threadId, cFunctor * pFunctor)
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

tResult ThreadCallerCreate()
{
   cAutoIPtr<IThreadCaller> p(new cThreadCaller);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IThreadCaller, static_cast<IThreadCaller*>(p));
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

class cFooStatic
{
public:
   static void SetFoo(ulong foo);
   static ulong gm_foo;
   static ulong gm_nSetFooCalls;
   static tThreadId gm_threadIdSetFooLastCalledFrom;
};
void cFooStatic::SetFoo(ulong foo)
{
   gm_foo = foo;
   gm_nSetFooCalls++;
   gm_threadIdSetFooLastCalledFrom = ThreadGetCurrentId();
}
ulong cFooStatic::gm_foo = 0;
ulong cFooStatic::gm_nSetFooCalls = 0;
tThreadId cFooStatic::gm_threadIdSetFooLastCalledFrom = 0;

class cReceiveThreadCallsThread : public cThread
{
public:
   virtual int Run();
};

int cReceiveThreadCallsThread::Run()
{
   UseGlobal(ThreadCaller);
   if (pThreadCaller->ThreadInit() != S_OK)
   {
      return -1;
   }
   for (;;)
   {
      if (pThreadCaller->ReceiveCalls(NULL) == S_OK)
      {
         break;
      }
   }
   if (pThreadCaller->ThreadTerm() != S_OK)
   {
      return -1;
   }
   return 0;
}

class cThreadFixture
{
public:
   cThreadFixture();
   ~cThreadFixture();
};

cThreadFixture::cThreadFixture()
{
   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadInit();
}

cThreadFixture::~cThreadFixture()
{
   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();
}

////////////////////////////////////////

TEST_F(cThreadFixture, ThreadCallerPostCall)
{
   cThread * pThread = new cReceiveThreadCallsThread;
   CHECK(pThread->Create());

   UseGlobal(ThreadCaller);

   static const ulong kMaxSleeps = 1000;
   ulong nSleeps = 0;
   while (pThreadCaller->ThreadIsInitialized(pThread->GetThreadId()) != S_OK)
   {
      ThreadSleep(100);
      nSleeps++;
      CHECK(nSleeps < kMaxSleeps);
   }

   tThreadId threadId = pThread->GetThreadId();
   ThreadSetName(threadId, "ReceiveThreadCallsThread");

   static const ulong kFoo = 4000;
   CHECK(cFooStatic::gm_foo != kFoo);
   CHECK_EQUAL(cFooStatic::gm_nSetFooCalls, 0);
   CHECK(cFooStatic::gm_threadIdSetFooLastCalledFrom != threadId);

   CHECK(pThreadCaller->PostCall(threadId, &cFooStatic::SetFoo, kFoo) == S_OK);
   pThread->Join();

   delete pThread, pThread = NULL;

   CHECK_EQUAL(cFooStatic::gm_foo, kFoo);
   CHECK_EQUAL(cFooStatic::gm_nSetFooCalls, 1);
   CHECK_EQUAL(cFooStatic::gm_threadIdSetFooLastCalledFrom, threadId);
}

#endif // HAVE_CPPUNITLITE2

////////////////////////////////////////////////////////////////////////////////
