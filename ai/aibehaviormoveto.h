////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIBEHAVIORMOVETO_H
#define INCLUDED_AIBEHAVIORMOVETO_H

#include "ai/aiapi.h"

#include "tech/statemachine.h"
#include "tech/vec3.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIBehaviorMoveTo
//

class cAIBehaviorMoveTo : public cComObject<IMPLEMENTS(IAIBehavior)>
                        , public cStateMachine<cAIBehaviorMoveTo, double>
{
public:
   cAIBehaviorMoveTo(const tVec3 & point);
   ~cAIBehaviorMoveTo();

   virtual tResult Update(IAIAgent * pAgent, double elapsedTime);

   void OnInitialStateUpdate(double);

private:
   void OnEnterMoving();
   void OnUpdateMoving(double elapsed);
   void OnExitMoving();

   void OnEnterArrived();
   void OnUpdateArrived(double elapsed);
   void OnExitArrived();

   tState m_movingState;
   tState m_arrivedState;

   tVec3 m_moveGoal;
   float m_lastDistSqr;

   cAutoIPtr<IAIAgentLocationProvider> m_pLocationProvider;
   cAutoIPtr<IAIAgentAnimationProvider> m_pAnimationProvider;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIBEHAVIORMOVETO_H
