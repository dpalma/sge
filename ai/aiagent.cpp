///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagent.h"

#include "tech/globalobj.h"
#include "tech/multivar.h"
#include "tech/vec3.h"

#include <algorithm>

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgent
//

////////////////////////////////////////

cAIAgent::cAIAgent(tAIAgentID id, IUnknown * pUnkOuter)
 : cComAggregableObject<IMPLEMENTS(IAIAgent)>(pUnkOuter)
 , m_id(id)
{
}

////////////////////////////////////////

cAIAgent::~cAIAgent()
{
   RemoveAllTasks();
}

////////////////////////////////////////

tResult cAIAgent::SetDefaultTask(IAIAgentTask * pTask)
{
   SafeRelease(m_pDefaultTask);
   m_pDefaultTask = CTAddRef(pTask);
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::GetDefaultTask(IAIAgentTask * * ppTask)
{
   return m_pDefaultTask.GetPointer(ppTask);
}

////////////////////////////////////////

tResult cAIAgent::AddTask(IAIAgentTask * pTask)
{
   if (pTask == NULL)
   {
      return E_POINTER;
   }
   m_taskQueue.push_back(CTAddRef(pTask));
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::RemoveTask(IAIAgentTask * pTask)
{
   if (pTask == NULL)
   {
      return E_POINTER;
   }
   if (!m_taskQueue.empty())
   {
      tTaskList::iterator iter = m_taskQueue.begin(), end = m_taskQueue.end();
      for (; iter != end; ++iter)
      {
         if (CTIsSameObject(pTask, *iter))
         {
            (*iter)->Release();
            m_taskQueue.erase(iter);
            return S_OK;
         }
      }
   }
   return S_FALSE;
}

////////////////////////////////////////

tResult cAIAgent::RemoveAllTasks()
{
   if (!m_taskQueue.empty())
   {
      std::for_each(m_taskQueue.begin(), m_taskQueue.end(), CTInterfaceMethod(&IUnknown::Release));
      m_taskQueue.clear();
      return S_OK;
   }
   return S_FALSE;
}

////////////////////////////////////////

tResult cAIAgent::GetActiveTask(IAIAgentTask * * ppTask)
{
   if (m_taskQueue.empty())
   {
      return GetDefaultTask(ppTask);
   }
   else
   {
      IAIAgentTask * pTask = m_taskQueue.front();
      if (pTask == NULL)
      {
         return E_FAIL; // but, this shouldn't ever happen
      }
      *ppTask = CTAddRef(pTask);
      return S_OK;
   }
}

////////////////////////////////////////

tAIAgentID cAIAgent::GetID() const
{
   return m_id;
}

////////////////////////////////////////

tResult cAIAgent::SetLocationProvider(IAIAgentLocationProvider * pLocationProvider)
{
   SafeRelease(m_pLocationProvider);
   m_pLocationProvider = CTAddRef(pLocationProvider);
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::GetLocationProvider(IAIAgentLocationProvider * * ppLocationProvider)
{
   return m_pLocationProvider.GetPointer(ppLocationProvider);
}

////////////////////////////////////////

tResult cAIAgent::SetAnimationProvider(IAIAgentAnimationProvider * pAnimationProvider)
{
   SafeRelease(m_pAnimationProvider);
   m_pAnimationProvider = CTAddRef(pAnimationProvider);
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::GetAnimationProvider(IAIAgentAnimationProvider * * ppAnimationProvider)
{
   return m_pAnimationProvider.GetPointer(ppAnimationProvider);
}

////////////////////////////////////////

tResult cAIAgent::Update(double time)
{
   cAutoIPtr<IAIAgentTask> pTask;
   if (!m_taskQueue.empty())
   {
      pTask = CTAddRef(m_taskQueue.front());
      if (pTask->Update(static_cast<IAIAgent*>(this), time) == S_AI_AGENT_TASK_DONE)
      {
         SafeRelease(m_taskQueue.front());
         m_taskQueue.pop_front();
      }
   }
   else if (!!m_pDefaultTask)
   {
      if (m_pDefaultTask->Update(static_cast<IAIAgent*>(this), time) == S_AI_AGENT_TASK_DONE)
      {
         SafeRelease(m_pDefaultTask);
      }
   }
   return S_OK;
}

////////////////////////////////////////

static bool GetVec3(IAIAgentMessage * pMessage, tVec3 * pVec)
{
   if (pMessage->GetArgumentCount() == 3)
   {
      cMultiVar args[3];
      for (int i = 0; i < _countof(args); ++i)
      {
         if (pMessage->GetArgument(i, &args[i]) != S_OK || !IsNumber(args[i]))
         {
            return false;
         }
      }

      *pVec = tVec3(args[0].ToFloat(), args[1].ToFloat(), args[2].ToFloat());
      return true;
   }

   return false;
}

tResult cAIAgent::HandleMessage(IAIAgentMessage * pMessage)
{
   Assert(pMessage != NULL);

   eAIAgentMessageType msgType = pMessage->GetMessageType();

   switch (msgType)
   {
      case kAIAMT_Stop:
      {
         RemoveAllTasks();
         cAutoIPtr<IAIAgentTask> pTask;
         if (AIAgentTaskStandCreate(&pTask) == S_OK)
         {
            SetDefaultTask(pTask);
         }
         break;
      }

      case kAIAMT_MoveTo:
      {
         RemoveAllTasks();
         tVec3 point;
         if (GetVec3(pMessage, &point))
         {
            cAutoIPtr<IAIAgentTask> pTask;
            if (AIAgentTaskMoveToCreate(point, &pTask) == S_OK)
            {
               SetDefaultTask(pTask);
            }
         }
         break;
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult AIAgentCreate(tAIAgentID id, IUnknown * pUnkOuter, IAIAgent * * ppAgent)
{
   if (id == 0)
   {
      return E_INVALIDARG;
   }

   if (ppAgent == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IAIAgent> pAgent(static_cast<IAIAgent *>(new cAIAgent(id, pUnkOuter)));
   if (!pAgent)
   {
      return E_OUTOFMEMORY;
   }

   UseGlobal(AIAgentMessageRouter);
   ErrorMsgIf(!pAIAgentMessageRouter, "No AI agent message router\n");
   if (!!pAIAgentMessageRouter)
   {
      pAIAgentMessageRouter->RegisterAgent(pAgent);
   }

   return pAgent.GetPointer(ppAgent);
}


///////////////////////////////////////////////////////////////////////////////
