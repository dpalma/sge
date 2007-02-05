////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENTBEHAVIORFOLLOWORDERS_H
#define INCLUDED_AIAGENTBEHAVIORFOLLOWORDERS_H

#include "ai/aiagentapi.h"

#include "tech/vec3.h"

#include <utility>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentBehaviorFollowOrders
//

class cAIAgentBehaviorFollowOrders : public cComObject<IMPLEMENTS(IAIAgentBehavior)>
{
public:
   cAIAgentBehaviorFollowOrders();
   ~cAIAgentBehaviorFollowOrders();

   virtual tResult HandleMessage(IAIAgent * pAgent, IAIAgentMessage * pMessage);
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENTBEHAVIORFOLLOWORDERS_H
