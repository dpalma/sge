///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scheduler.h"

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Scheduler);

#define LocalMsg(msg)            DebugMsgEx(Scheduler,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(Scheduler,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(Scheduler,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(Scheduler,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(Scheduler,msg,(a),(b),(c),(d))

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
      std::deque<ITask *>::iterator iter = m_renderTaskQueue.begin();
      for (; iter != m_renderTaskQueue.end(); iter++)
      {
         (*iter)->Release();
      }
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
   std::deque<ITask *>::iterator iter = m_renderTaskQueue.begin();
   for (; iter != m_renderTaskQueue.end(); iter++)
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

   LocalMsg4("Frame %d: %d time tasks, %d frame tasks, %d render tasks\n",
      m_clock.GetFrameCount(), m_timeTaskQueue.size(),
      m_frameTaskQueue.size(), m_renderTaskQueue.size());

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
      std::deque<ITask *>::iterator iter = m_renderTaskQueue.begin();
      for (; iter != m_renderTaskQueue.end(); iter++)
      {
         (*iter)->Execute(m_clock.GetFrameCount());
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

tResult SchedulerCreate()
{
   cAutoIPtr<IScheduler> p(static_cast<IScheduler*>(new cScheduler));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IScheduler, p);
}

///////////////////////////////////////////////////////////////////////////////
