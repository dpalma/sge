///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aibehaviorwander.h"

#include "tech/statemachinetem.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIBehaviorWander
//

////////////////////////////////////////

cAIBehaviorWander::cAIBehaviorWander()
 : m_idleState(&cAIBehaviorWander::OnEnterIdle, &cAIBehaviorWander::OnIdle, &cAIBehaviorWander::OnExitIdle)
 , m_movingState(&cAIBehaviorWander::OnEnterMoving, &cAIBehaviorWander::OnMoving, &cAIBehaviorWander::OnExitMoving)
{
}

////////////////////////////////////////

cAIBehaviorWander::~cAIBehaviorWander()
{
}

////////////////////////////////////////

tResult cAIBehaviorWander::Update(IAIAgent * pAgent, double elapsedTime)
{
   SafeRelease(m_pLocationProvider);
   if (pAgent != NULL)
   {
      pAgent->GetLocationProvider(&m_pLocationProvider);
   }

   cStateMachine<cAIBehaviorWander, double>::Update(elapsedTime);

   return S_AI_BEHAVIOR_CONTINUE;
}

////////////////////////////////////////

void cAIBehaviorWander::MoveTo(const tVec3 & point)
{
   m_moveGoal = point;

   //SafeRelease(m_pRender);
   //m_pRender = CTAddRef(pRender);

   GotoState(&m_movingState);
}

////////////////////////////////////////

void cAIBehaviorWander::Stop()
{
   //SafeRelease(m_pPosition);
   GotoState(&m_idleState);
}

////////////////////////////////////////

void cAIBehaviorWander::OnEnterIdle()
{
   //if (!!m_pRender)
   //{
   //   m_pRender->SetAnimation(kMAT_Idle);
   //}
}

////////////////////////////////////////

void cAIBehaviorWander::OnIdle(double elapsed)
{
}

////////////////////////////////////////

void cAIBehaviorWander::OnExitIdle()
{
}

////////////////////////////////////////

void cAIBehaviorWander::OnEnterMoving()
{
   //if (!!m_pRender)
   //{
   //   m_pRender->SetAnimation(kMAT_Walk);
   //}
}

////////////////////////////////////////

void cAIBehaviorWander::OnMoving(double elapsed)
{
   if (!m_pLocationProvider)
   {
      return;
   }

   tVec3 curPos;
   if (m_pLocationProvider->GetPosition(&curPos) != S_OK)
   {
      return;
   }

   if (AlmostEqual(curPos.x, m_moveGoal.x)
      && AlmostEqual(curPos.z, m_moveGoal.z))
   {
      GotoState(&m_idleState);
//      Update(elapsed); // force idle immediately
   }
   else
   {
      tVec3 dir = m_moveGoal - curPos;
      dir.Normalize();
      curPos += (dir * 10.0f * static_cast<float>(elapsed));
      m_pLocationProvider->SetPosition(curPos);
   }
}

////////////////////////////////////////

void cAIBehaviorWander::OnExitMoving()
{
}


///////////////////////////////////////////////////////////////////////////////
