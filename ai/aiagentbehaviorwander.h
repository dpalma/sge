////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENTBEHAVIORWANDER_H
#define INCLUDED_AIAGENTBEHAVIORWANDER_H

#include "ai/aiagentapi.h"

#include "tech/statemachine.h"
#include "tech/vec3.h"

#include <utility>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentBehaviorWander
//

typedef std::pair<IAIAgent*, IAIAgentMessage*> tAgentMessagePair;

class cAIAgentBehaviorWander : public cComObject<IMPLEMENTS(IAIAgentBehavior)>
                             , public cStateMachine<cAIAgentBehaviorWander, const tAgentMessagePair &>
{
   typedef cStateMachine<cAIAgentBehaviorWander, const tAgentMessagePair &> tStateMachineBase;

public:
   cAIAgentBehaviorWander();
   ~cAIAgentBehaviorWander();

   virtual tResult HandleMessage(IAIAgent * pAgent, IAIAgentMessage * pMessage);

private:
   void MoveTo(const tVec3 & point);

   void OnEnterIdle();
   void OnExitIdle();
   void OnHandleMessageIdle(const tAgentMessagePair &);

   void OnEnterMoving();
   void OnExitMoving();
   void OnHandleMessageMoving(const tAgentMessagePair &);

   tState m_idleState;
   tState m_movingState;

   tVec3 m_moveGoal;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENTBEHAVIORWANDER_H
