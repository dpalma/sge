////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIBEHAVIORWANDER_H
#define INCLUDED_AIBEHAVIORWANDER_H

#include "ai/aiapi.h"

#include "tech/statemachine.h"
#include "tech/vec3.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIBehaviorWander
//

class cAIBehaviorWander : public cComObject<IMPLEMENTS(IAIBehavior)>
                        , public cStateMachine<cAIBehaviorWander, double>
{
public:
   cAIBehaviorWander();
   ~cAIBehaviorWander();

   virtual tResult Update(IAIAgent * pAgent, double elapsedTime);

private:
   void MoveTo(const tVec3 & point);
   void Stop();

   void OnEnterIdle();
   void OnIdle(double elapsed);
   void OnExitIdle();

   void OnEnterMoving();
   void OnMoving(double elapsed);
   void OnExitMoving();

   tState m_idleState;
   tState m_movingState;

   tVec3 m_moveGoal;

   cAutoIPtr<IAIAgentLocationProvider> m_pLocationProvider;
   cAutoIPtr<IAIAgentAnimationProvider> m_pAnimationProvider;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIBEHAVIORWANDER_H
