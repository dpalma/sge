///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagent.h"
#include "aiagentmessage.h"

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
   std::for_each(m_behaviorStack.begin(), m_behaviorStack.end(), CTInterfaceMethod(&IAIAgentBehavior::Release));
   m_behaviorStack.clear();
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

tResult cAIAgent::PushBehavior(IAIAgentBehavior * pBehavior)
{
   if (pBehavior == NULL)
   {
      return E_POINTER;
   }

   if (!m_behaviorStack.empty())
   {
      cAutoIPtr<IAIAgentBehavior> pCurrentBehavior(CTAddRef(m_behaviorStack.front()));
      cMultiVar msgArg(pCurrentBehavior);
      cAIAgentMessageNoSelfDelete msg(m_id, m_id, 0, kAIAMT_BehaviorPause, 1, &msgArg);
      HandleMessage(static_cast<IAIAgentMessage*>(&msg));
   }

   {
      cMultiVar msgArg(pBehavior);
      cAIAgentMessageNoSelfDelete msg(m_id, m_id, 0, kAIAMT_BehaviorBegin, 1, &msgArg);
      HandleMessage(static_cast<IAIAgentMessage*>(&msg));
   }

   m_behaviorStack.push_front(CTAddRef(pBehavior));
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::PopBehavior()
{
   if (m_behaviorStack.empty())
   {
      return E_FAIL;
   }

   {
      cMultiVar msgArg(CTAddRef(m_behaviorStack.front()));
      cAIAgentMessageNoSelfDelete msg(m_id, m_id, 0, kAIAMT_BehaviorEnd, 1, &msgArg);
      HandleMessage(static_cast<IAIAgentMessage*>(&msg));
   }

   IAIAgentBehavior * pBehavior = m_behaviorStack.front();
   m_behaviorStack.pop_front();
   SafeRelease(pBehavior);

   if (!m_behaviorStack.empty())
   {
      cAutoIPtr<IAIAgentBehavior> pNextBehavior(CTAddRef(m_behaviorStack.front()));
      cMultiVar msgArg(CTAddRef(pNextBehavior));
      cAIAgentMessageNoSelfDelete msg(m_id, m_id, 0, kAIAMT_BehaviorResume, 1, &msgArg);
      HandleMessage(static_cast<IAIAgentMessage*>(&msg));
   }

   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::GetActiveBehavior(IAIAgentBehavior * * ppBehavior)
{
   if (!m_behaviorStack.empty())
   {
      IAIAgentBehavior * pBehavior = m_behaviorStack.front();
      if (pBehavior == NULL)
      {
         return E_FAIL; // but, this shouldn't ever happen
      }
      *ppBehavior = CTAddRef(pBehavior);
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cAIAgent::SetActiveTask(IAIAgentTask * pTask)
{
   SafeRelease(m_pActiveTask);
   m_pActiveTask = CTAddRef(pTask);
   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::GetActiveTask(IAIAgentTask * * ppTask)
{
   if (ppTask == NULL)
   {
      return E_POINTER;
   }
   return m_pActiveTask.GetPointer(ppTask);
}

////////////////////////////////////////

tResult cAIAgent::Update(double time)
{
   if (!!m_pActiveTask)
   {
      tResult result = m_pActiveTask->Update(static_cast<IAIAgent*>(this), time);

      if (FAILED(result))
      {
         return result;
      }
      else if (result == S_AI_DONE)
      {
         cMultiVar msgArg(m_pActiveTask);
         cAIAgentMessageNoSelfDelete msg(m_id, m_id, time, kAIAMT_TaskDone, 1, &msgArg);
         HandleMessage(static_cast<IAIAgentMessage*>(&msg));

         SafeRelease(m_pActiveTask);
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cAIAgent::HandleMessage(IAIAgentMessage * pMessage)
{
   Assert(pMessage != NULL);

   cAutoIPtr<IAIAgentBehavior> pBehavior;
   if (GetActiveBehavior(&pBehavior) == S_OK)
   {
      if (pBehavior->HandleMessage(static_cast<IAIAgent*>(this), pMessage) == S_AI_DONE)
      {
         PopBehavior();
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
