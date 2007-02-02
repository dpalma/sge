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

class cAIAgent : public cComObject<IMPLEMENTS(IAIAgent)>
{
public:
   cAIAgent(tAIAgentID id);
   ~cAIAgent();

   virtual tResult SetDefaultBehavior(IAIBehavior * pBehavior);
   virtual tResult GetDefaultBehavior(IAIBehavior * * ppBehavior);

   virtual tResult PushBehavior(IAIBehavior * pBehavior);
   virtual tResult PopBehavior();

   virtual tResult GetActiveBehavior(IAIBehavior * * ppBehavior);

   virtual tResult SetLocationProvider(IAIAgentLocationProvider * pLocationProvider);
   virtual tResult GetLocationProvider(IAIAgentLocationProvider * * ppLocationProvider);

   virtual tResult SetAnimationProvider(IAIAgentAnimationProvider * pAnimationProvider);
   virtual tResult GetAnimationProvider(IAIAgentAnimationProvider * * ppAnimationProvider);

   virtual tResult HandleMessage(IAIAgentMessage * pMessage);

private:
   tAIAgentID m_id;

   cAutoIPtr<IAIBehavior> m_pDefaultBehavior;

   typedef std::list<IAIBehavior*> tAIBehaviorList;
   tAIBehaviorList m_behaviorStack;

   cAutoIPtr<IAIAgentLocationProvider> m_pLocationProvider;
   cAutoIPtr<IAIAgentAnimationProvider> m_pAnimationProvider;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENT_H
