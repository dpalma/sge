///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagenttaskstand.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentTaskStand
//

////////////////////////////////////////

cAIAgentTaskStand::cAIAgentTaskStand()
 : m_bAnimStarted(false)
{
}

////////////////////////////////////////

cAIAgentTaskStand::~cAIAgentTaskStand()
{
}

////////////////////////////////////////

tResult cAIAgentTaskStand::Update(IAIAgent * pAgent, double elapsedTime)
{
   if (!m_bAnimStarted)
   {
      m_bAnimStarted = true;

      cAutoIPtr<IAIAgentAnimationProvider> pAnimationProvider;
      if (pAgent != NULL && pAgent->GetAnimationProvider(&pAnimationProvider) == S_OK)
      {
         pAnimationProvider->RequestAnimation(kAIAA_Fidget);
      }
   }

   return S_AI_AGENT_TASK_CONTINUE;
}

////////////////////////////////////////

tResult AIAgentTaskStandCreate(IAIAgentTask * * ppTask)
{
   if (ppTask == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IAIAgentTask> pTask(static_cast<IAIAgentTask *>(new cAIAgentTaskStand));
   if (!pTask)
   {
      return E_OUTOFMEMORY;
   }
   return pTask.GetPointer(ppTask);
}


///////////////////////////////////////////////////////////////////////////////
