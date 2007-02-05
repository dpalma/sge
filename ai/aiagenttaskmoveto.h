////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENTTASKMOVETO_H
#define INCLUDED_AIAGENTTASKMOVETO_H

#include "ai/aiagentapi.h"

#include "tech/statemachine.h"
#include "tech/vec3.h"

#include <utility>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentTaskMoveTo
//

typedef std::pair<IAIAgent *, double> tAgentDoublePair;

class cAIAgentTaskMoveTo : public cComObject<IMPLEMENTS(IAIAgentTask)>
                         , public cStateMachine<cAIAgentTaskMoveTo, const tAgentDoublePair &>
{
public:
   cAIAgentTaskMoveTo(const tVec3 & point);
   ~cAIAgentTaskMoveTo();

   virtual tResult Update(IAIAgent * pAgent, double time);

private:
   void OnEnterMoving();
   void OnExitMoving();
   void OnUpdateMoving(const tAgentDoublePair &);

   void OnEnterArrived();
   void OnExitArrived();
   void OnUpdateArrived(const tAgentDoublePair &);

   tState m_movingState;
   tState m_arrivedState;

   tVec3 m_moveGoal;
   float m_lastDistSqr;

   bool m_bJustStarted;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENTTASKMOVETO_H
