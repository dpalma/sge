///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aibehaviorstand.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIBehaviorStand
//

////////////////////////////////////////

cAIBehaviorStand::cAIBehaviorStand()
 : m_bAnimStarted(false)
{
}

////////////////////////////////////////

cAIBehaviorStand::~cAIBehaviorStand()
{
}

////////////////////////////////////////

tResult cAIBehaviorStand::Update(IAIAgent * pAgent, double elapsedTime)
{
   if (!m_bAnimStarted)
   {
      m_bAnimStarted = true;

      cAutoIPtr<IAIAgentAnimationProvider> pAnimationProvider;
      if (pAgent != NULL && pAgent->GetAnimationProvider(&pAnimationProvider) == S_OK)
      {
         pAnimationProvider->SetAnimation(kAIAgentAnimIdle);
      }
   }

   return S_AI_BEHAVIOR_CONTINUE;
}

////////////////////////////////////////

tResult AIBehaviorStandCreate(IAIBehavior * * ppBehavior)
{
   if (ppBehavior == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IAIBehavior> pBehavior(static_cast<IAIBehavior *>(new cAIBehaviorStand));
   if (!pBehavior)
   {
      return E_OUTOFMEMORY;
   }
   return pBehavior.GetPointer(ppBehavior);
}


///////////////////////////////////////////////////////////////////////////////
