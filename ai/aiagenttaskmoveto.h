////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENTTASKMOVETO_H
#define INCLUDED_AIAGENTTASKMOVETO_H

#include "ai/aiagentapi.h"

#include "tech/statemachine.h"
#include "tech/vec3.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentTaskMoveTo
//

class cAIAgentTaskMoveTo : public cComObject<IMPLEMENTS(IAIAgentTask)>
                         , public cStateMachine<cAIAgentTaskMoveTo, double>
{
public:
   cAIAgentTaskMoveTo(const tVec3 & point);
   ~cAIAgentTaskMoveTo();

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

#endif // !INCLUDED_AIAGENTTASKMOVETO_H
