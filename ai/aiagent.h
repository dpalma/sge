////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENT_H
#define INCLUDED_AIAGENT_H

#include "ai/aiagentapi.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgent
//

class cAIAgent : public cComAggregableObject<IMPLEMENTS(IAIAgent)>
{
public:
   cAIAgent(tAIAgentID id, IUnknown * pUnkOuter);
   ~cAIAgent();

   virtual tAIAgentID GetID() const;

   virtual tResult SetLocationProvider(IAIAgentLocationProvider * pLocationProvider);
   virtual tResult GetLocationProvider(IAIAgentLocationProvider * * ppLocationProvider);

   virtual tResult SetAnimationProvider(IAIAgentAnimationProvider * pAnimationProvider);
   virtual tResult GetAnimationProvider(IAIAgentAnimationProvider * * ppAnimationProvider);

   virtual tResult PushBehavior(IAIAgentBehavior * pBehavior);
   virtual tResult PopBehavior();

   tResult GetActiveBehavior(IAIAgentBehavior * * ppBehavior);

   virtual tResult SetActiveTask(IAIAgentTask * pTask);
   virtual tResult GetActiveTask(IAIAgentTask * * ppTask);

   virtual tResult Update(double time);

   virtual tResult HandleMessage(IAIAgentMessage * pMessage);

private:
   tAIAgentID m_id;

   cAutoIPtr<IAIAgentLocationProvider> m_pLocationProvider;
   cAutoIPtr<IAIAgentAnimationProvider> m_pAnimationProvider;

   typedef std::list<IAIAgentBehavior*> tBehaviorList;
   tBehaviorList m_behaviorStack;

   cAutoIPtr<IAIAgentTask> m_pActiveTask;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENT_H
