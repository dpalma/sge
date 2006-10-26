////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "threadcaller.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include "tech/dbgalloc.h" // must be last header

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
      {
         tThreadCalls::iterator iter = m_calls.begin();
         tThreadCalls::iterator end = m_calls.end();
         for (; iter != end; iter++)
         {
            delete iter->second;
         }

         m_calls.clear();
      }

      {
         tThreadInfoMap::iterator iter = m_threadInfoMap.begin();
         tThreadInfoMap::iterator end = m_threadInfoMap.end();
         for (; iter != end; ++iter)
         {
            sThreadInfo * pThreadInfo = iter->second;
            pThreadInfo->callEvent.Pulse();
            delete pThreadInfo;
         }

         m_threadInfoMap.clear();
      }
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

      {
         tThreadInfoMap::iterator f = m_threadInfoMap.find(threadId);
         if (f != m_threadInfoMap.end())
         {
            // Thread already initialized
            f->second->initCount += 1;
            return S_FALSE;
         }
      }

      sThreadInfo * pThreadInfo = new sThreadInfo;
      if (pThreadInfo == NULL)
      {
         return E_OUTOFMEMORY;
      }

      pThreadInfo->initCount = 1;

      if (!pThreadInfo->callEvent.Create())
      {
         delete pThreadInfo;
         return E_FAIL;
      }

      m_threadInfoMap.insert(std::make_pair(threadId, pThreadInfo));

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

      {
         tThreadInfoMap::iterator f = m_threadInfoMap.find(threadId);
         if (f != m_threadInfoMap.end())
         {
            sThreadInfo * pThreadInfo = f->second;
            pThreadInfo->callEvent.Pulse();
            pThreadInfo->initCount -= 1;
            if (pThreadInfo->initCount == 0)
            {
               delete pThreadInfo;
               m_threadInfoMap.erase(f);
            }
            return S_OK;
         }
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
      tThreadInfoMap::iterator f = m_threadInfoMap.find(threadId);
      if (f != m_threadInfoMap.end())
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

      sThreadInfo * pThreadInfo = NULL;

      {
         tThreadInfoMap::iterator f = m_threadInfoMap.find(threadId);
         if (f != m_threadInfoMap.end())
         {
            pThreadInfo = f->second;
         }
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

      pThreadInfo->callEvent.Pulse();

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

#ifdef HAVE_UNITTESTPP

class cFooStatic
{
public:
   static void SetFoo(int foo);
   static int gm_foo;
   static int gm_nSetFooCalls;
   static tThreadId gm_threadIdSetFooLastCalledFrom;
};
void cFooStatic::SetFoo(int foo)
{
   gm_foo = foo;
   gm_nSetFooCalls++;
   gm_threadIdSetFooLastCalledFrom = ThreadGetCurrentId();
}
int cFooStatic::gm_foo = 0;
int cFooStatic::gm_nSetFooCalls = 0;
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

TEST_FIXTURE(cThreadFixture, ThreadCallerPostCall)
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

   static const int kFoo = 4000;
   CHECK(cFooStatic::gm_foo != kFoo);
   CHECK_EQUAL(cFooStatic::gm_nSetFooCalls, 0);
   CHECK(cFooStatic::gm_threadIdSetFooLastCalledFrom != threadId);

   CHECK(pThreadCaller->PostCall(threadId, &cFooStatic::SetFoo, kFoo) == S_OK);
   pThread->Join();

   delete pThread, pThread = NULL;

   CHECK_EQUAL(cFooStatic::gm_foo, kFoo);
   CHECK_EQUAL(cFooStatic::gm_nSetFooCalls, 1);
   CHECK(cFooStatic::gm_threadIdSetFooLastCalledFrom == threadId);
}

#endif // HAVE_UNITTESTPP

////////////////////////////////////////////////////////////////////////////////
