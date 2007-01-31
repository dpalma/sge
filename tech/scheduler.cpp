///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scheduler.h"

#include "tech/techtime.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Scheduler);

#define LocalMsg(msg)            DebugMsgEx(Scheduler,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(Scheduler,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(Scheduler,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(Scheduler,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(Scheduler,msg,(a),(b),(c),(d))
#define LocalMsg5(msg,a,b,c,d,e) DebugMsgEx5(Scheduler,msg,(a),(b),(c),(d),(e))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(Scheduler,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(Scheduler,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(Scheduler,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(Scheduler,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(Scheduler,(cond),msg,(a),(b),(c),(d))

///////////////////////////////////////////////////////////////////////////////

static const double kNoExpiration = 0;


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sTaskInfo
//

////////////////////////////////////////

sTaskInfo::sTaskInfo()
 : pTask(NULL)
 , start(0)
 , period(0)
 , expiration(0)
 , next(0)
{
}

////////////////////////////////////////

sTaskInfo::sTaskInfo(const sTaskInfo & other)
 : pTask(other.pTask)
 , start(other.start)
 , period(other.period)
 , expiration(other.expiration)
 , next(other.next)
{
}

////////////////////////////////////////

sTaskInfo::~sTaskInfo()
{
}

////////////////////////////////////////

const sTaskInfo & sTaskInfo::operator =(const sTaskInfo & other)
{
   pTask = other.pTask;
   start = other.start;
   period = other.period;
   expiration = other.expiration;
   next = other.next;
   return *this;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScheduler
//

////////////////////////////////////////

cScheduler::cScheduler()
 : m_lockRenderTaskQueue(0)
{
}

////////////////////////////////////////

cScheduler::~cScheduler()
{
}

////////////////////////////////////////

tResult cScheduler::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cScheduler::Term()
{
   {
      std::for_each(m_renderTaskQueue.begin(), m_renderTaskQueue.end(), CTInterfaceMethod(&ITask::Release));
      m_renderTaskQueue.clear();
   }

   while (!m_frameTaskQueue.empty())
   {
      sTaskInfo * pTaskInfo = m_frameTaskQueue.top();
      m_frameTaskQueue.pop();
      delete pTaskInfo;
   }

   while (!m_timeTaskQueue.empty())
   {
      sTaskInfo * pTaskInfo = m_timeTaskQueue.top();
      m_timeTaskQueue.pop();
      delete pTaskInfo;
   }

   return S_OK;
}

////////////////////////////////////////

void cScheduler::Start()
{
   m_clock.Start();
}

////////////////////////////////////////

void cScheduler::Stop()
{
   m_clock.Stop();
}

////////////////////////////////////////

bool cScheduler::IsRunning() const
{
   return m_clock.IsRunning();
}

////////////////////////////////////////

tResult cScheduler::AddRenderTask(ITask * pTask)
{
   if (pTask == NULL)
   {
      return E_POINTER;
   }

   if (m_lockRenderTaskQueue != 0)
   {
      return E_FAIL;
   }

   LocalMsg1("Adding render task %p\n", pTask);

   m_renderTaskQueue.push_back(CTAddRef(pTask));
   return S_OK;
}

////////////////////////////////////////

tResult cScheduler::RemoveRenderTask(ITask * pTask)
{
   if (pTask == NULL)
   {
      return E_POINTER;
   }

   if (m_lockRenderTaskQueue != 0)
   {
      return E_FAIL;
   }

   std::deque<ITask *>::iterator iter = m_renderTaskQueue.begin(), end = m_renderTaskQueue.end();
   for (; iter != end; ++iter)
   {
      if (CTIsSameObject(pTask, *iter))
      {
         (*iter)->Release();
         m_renderTaskQueue.erase(iter);
         return S_OK;
      }
   }

   return S_FALSE;
}

////////////////////////////////////////

static tResult AddTask(tTaskQueue * pQueue, ITask * pTask, double start, double period, double duration)
{
   Assert(pQueue != NULL);

   if (period == 0)
   {
      return E_INVALIDARG;
   }

   if (pTask == NULL)
   {
      return E_POINTER;
   }

   sTaskInfo * pTaskInfo = new sTaskInfo;
   if (pTaskInfo == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pTaskInfo->pTask = CTAddRef(pTask);
   pTaskInfo->start = start;
   pTaskInfo->period = period;
   if (duration == 0)
   {
      pTaskInfo->expiration = kNoExpiration;
   }
   else
   {
      pTaskInfo->expiration = start + duration - 1;
   }
   pTaskInfo->next = start;

   pQueue->push(pTaskInfo);
   return S_OK;
}

static tResult RemoveTask(tTaskQueue * pQueue, ITask * pTask)
{
   if (pTask == NULL || pQueue == NULL)
   {
      return E_POINTER;
   }

   bool bFound = false;
   tTaskQueue newQueue;

   while (!pQueue->empty())
   {
      sTaskInfo * pTaskInfo = pQueue->top();
      pQueue->pop();

      if (CTIsSameObject(pTask, pTaskInfo->pTask))
      {
         delete pTaskInfo;
         bFound = true;
      }
      else
      {
         newQueue.push(pTaskInfo);
      }
   }

   while (!newQueue.empty())
   {
      sTaskInfo * pTaskInfo = newQueue.top();
      newQueue.pop();

      pQueue->push(pTaskInfo);
   }

   return bFound ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cScheduler::AddFrameTask(ITask * pTask, ulong start, ulong period, ulong duration)
{
   LocalMsg1("Adding frame-based task %p\n", pTask);
   return AddTask(&m_frameTaskQueue, pTask, static_cast<double>(start), static_cast<double>(period), static_cast<double>(duration));
}

////////////////////////////////////////

tResult cScheduler::RemoveFrameTask(ITask * pTask)
{
   LocalMsg1("Removing frame-based task %p\n", pTask);
   return RemoveTask(&m_frameTaskQueue, pTask);
}

////////////////////////////////////////

tResult cScheduler::AddTimeTask(ITask * pTask, double start, double period, double duration)
{
   LocalMsg1("Adding time-based task %p\n", pTask);
   return AddTask(&m_timeTaskQueue, pTask, start, period, duration);
}

////////////////////////////////////////

tResult cScheduler::RemoveTimeTask(ITask * pTask)
{
   LocalMsg1("Removing time-based task %p\n", pTask);
   return RemoveTask(&m_timeTaskQueue, pTask);
}

////////////////////////////////////////

void cScheduler::NextFrame()
{
   m_clock.BeginFrame();

   LocalMsg5("Frame %d, Time %f: %d time tasks, %d frame tasks, %d render tasks\n",
      m_clock.GetFrameCount(), m_clock.GetFrameStart(),
      m_timeTaskQueue.size(), m_frameTaskQueue.size(), m_renderTaskQueue.size());

   // Run time-based tasks
   while (!m_timeTaskQueue.empty())
   {
      sTaskInfo * pTaskInfo = m_timeTaskQueue.top();
      if (pTaskInfo->next <= m_clock.GetFrameEnd())
      {
         m_timeTaskQueue.pop();

         m_clock.AdvanceTo(pTaskInfo->next);

         if (pTaskInfo->pTask->Execute(m_clock.GetSimTime()) != S_OK)
         {
            delete pTaskInfo;
            continue;
         }

         pTaskInfo->next += pTaskInfo->period;

         if (pTaskInfo->expiration == kNoExpiration ||
            pTaskInfo->expiration >= pTaskInfo->next)
         {
            // Re-insert with updated time
            m_timeTaskQueue.push(pTaskInfo);
         }
         else
         {
            LocalMsg2("Time task %p expiring at %f\n", pTaskInfo->pTask, pTaskInfo->expiration);
            delete pTaskInfo;
         }
      }
      else
      {
         break;
      }
   }

   m_clock.EndFrame();

   // Run frame tasks
   while (!m_frameTaskQueue.empty())
   {
      sTaskInfo * pTaskInfo = m_frameTaskQueue.top();
      if (pTaskInfo->next <= m_clock.GetFrameCount())
      {
         LocalMsg2("Running frame task %p: time = %f\n", pTaskInfo->pTask, pTaskInfo->next);

         m_frameTaskQueue.pop();

         if (pTaskInfo->pTask->Execute(m_clock.GetSimTime()) != S_OK)
         {
            delete pTaskInfo;
            continue;
         }

         pTaskInfo->next += pTaskInfo->period;

         if (pTaskInfo->expiration == kNoExpiration ||
            pTaskInfo->expiration >= pTaskInfo->next)
         {
            // Re-insert with updated time
            m_frameTaskQueue.push(pTaskInfo);
         }
         else
         {
            LocalMsg2("Frame task %p expiring at %f\n", pTaskInfo->pTask, pTaskInfo->expiration);
            delete pTaskInfo;
         }
      }
      else
      {
         break;
      }
   }

   // Run render tasks
   {
      ++m_lockRenderTaskQueue;
      std::deque<ITask *>::iterator iter = m_renderTaskQueue.begin(), end = m_renderTaskQueue.end();
      for (; iter != end; ++iter)
      {
         if ((*iter)->Execute(m_clock.GetFrameCount()) != S_OK)
         {
            iter = m_renderTaskQueue.erase(iter);
         }
      }
      --m_lockRenderTaskQueue;
   }
}

///////////////////////////////////////////////////////////////////////////////

tResult SchedulerCreate()
{
   cAutoIPtr<IScheduler> pScheduler(static_cast<IScheduler*>(new cScheduler));
   if (!pScheduler)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IScheduler, pScheduler);
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

class cCounterTask : public cComObject<IMPLEMENTS(ITask)>
{
public:
   cCounterTask() : m_count(0) {}

   virtual tResult Execute(double time)
   {
      ++m_count;
      return S_OK;
   }

   int GetCount() const { return m_count; }

private:
   int m_count;
};

enum eSchedulerTaskType
{
   kRenderTask, kFrameTask, kTimeTask
};

class cRemoveSelfTask : public cComObject<IMPLEMENTS(ITask)>
{
public:
   cRemoveSelfTask(IScheduler * pScheduler, eSchedulerTaskType taskType)
      : m_pScheduler(pScheduler), m_taskType(taskType) {}

   virtual tResult Execute(double time)
   {
      if (m_taskType == kRenderTask)
      {
         m_pScheduler->RemoveRenderTask(static_cast<ITask*>(this));
      }
      else if (m_taskType == kFrameTask)
      {
         m_pScheduler->RemoveFrameTask(static_cast<ITask*>(this));
      }
      else if (m_taskType == kTimeTask)
      {
         m_pScheduler->RemoveTimeTask(static_cast<ITask*>(this));
      }
      return S_OK;
   }

private:
   IScheduler * m_pScheduler;
   eSchedulerTaskType m_taskType;
};

class cSchedulerTests
{
public:
   cSchedulerTests();
   ~cSchedulerTests();

   IScheduler * AccessScheduler() { return static_cast<IScheduler*>(m_pScheduler); }
   const IScheduler * AccessScheduler() const { return static_cast<const IScheduler*>(m_pScheduler); }

private:
   cAutoIPtr<cScheduler> m_pScheduler;
};

cSchedulerTests::cSchedulerTests()
{
   SafeRelease(m_pScheduler);
   m_pScheduler = new cScheduler;
   m_pScheduler->Init();
}

cSchedulerTests::~cSchedulerTests()
{
   if (!!m_pScheduler)
   {
      m_pScheduler->Term();
      SafeRelease(m_pScheduler);
   }
}

TEST_FIXTURE(cSchedulerTests, SchedulerFrameTaskExpiration)
{
   static const ulong kFrameTaskPeriod = 2;
   static const ulong kFrameTaskDuration = 6;
   static const ulong kMinFrames = static_cast<ulong>(kFrameTaskDuration / kFrameTaskPeriod);

   cAutoIPtr<cCounterTask> pFrameTask(new cCounterTask);

	CHECK(AccessScheduler()->AddFrameTask(pFrameTask, 0, kFrameTaskPeriod, kFrameTaskDuration) == S_OK);

   // run the scheduler for at least as many frames as it takes to expire the task
   AccessScheduler()->Start();
   for (ulong nFrames = 0; nFrames < (kMinFrames + 2); ++nFrames)
   {
      AccessScheduler()->NextFrame();
   }
   AccessScheduler()->Stop();

   CHECK_EQUAL(static_cast<int>(kFrameTaskDuration / kFrameTaskPeriod), pFrameTask->GetCount());
}

// Ensure nothing bad happens when tasks attempt to remove themselves during an update
TEST_FIXTURE(cSchedulerTests, SchedulerRemoveTaskDuringFrame)
{
   cAutoIPtr<ITask> pFrameTask(new cRemoveSelfTask(AccessScheduler(), kFrameTask));
   cAutoIPtr<ITask> pRenderTask(new cRemoveSelfTask(AccessScheduler(), kRenderTask));

	CHECK(AccessScheduler()->AddFrameTask(pFrameTask, 0, 1, 0) == S_OK);
	CHECK(AccessScheduler()->AddRenderTask(pRenderTask) == S_OK);

   AccessScheduler()->Start();
   AccessScheduler()->NextFrame();
   AccessScheduler()->Stop();
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
