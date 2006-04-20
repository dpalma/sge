///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scheduler.h"

#include "dbgalloc.h" // must be last header


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
      delete m_frameTaskQueue.top();
      m_frameTaskQueue.pop();
   }

   while (!m_timeTaskQueue.empty())
   {
      delete m_timeTaskQueue.top();
      m_timeTaskQueue.pop();
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

////////////////////////////////////////

tResult cScheduler::AddFrameTask(ITask * pTask, ulong start, ulong period, ulong duration)
{
   return AddTask(&m_frameTaskQueue, pTask, static_cast<double>(start), static_cast<double>(period), static_cast<double>(duration));
}

////////////////////////////////////////

tResult cScheduler::RemoveFrameTask(ITask * pTask)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cScheduler::AddTimeTask(ITask * pTask, double start, double period, double duration)
{
   return AddTask(&m_timeTaskQueue, pTask, start, period, duration);
}

////////////////////////////////////////

tResult cScheduler::RemoveTimeTask(ITask * pTask)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

void cScheduler::NextFrame()
{
   m_clock.BeginFrame();

   // Run time-based tasks
   while (!m_timeTaskQueue.empty())
   {
      sTaskInfo * pTaskInfo = m_timeTaskQueue.top();
      if (pTaskInfo->next <= m_clock.GetFrameEnd())
      {
         m_timeTaskQueue.pop();

         m_clock.AdvanceTo(pTaskInfo->next);

         pTaskInfo->pTask->Execute(m_clock.GetSimTime());
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
         m_frameTaskQueue.pop();

         pTaskInfo->pTask->Execute(m_clock.GetSimTime());
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
