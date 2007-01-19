///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aibehaviormoveto.h"

#include "tech/statemachinetem.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIBehaviorMoveTo
//

////////////////////////////////////////

cAIBehaviorMoveTo::cAIBehaviorMoveTo(const tVec3 & point)
 : m_movingState(&cAIBehaviorMoveTo::OnEnterMoving, &cAIBehaviorMoveTo::OnUpdateMoving, &cAIBehaviorMoveTo::OnExitMoving)
 , m_arrivedState(&cAIBehaviorMoveTo::OnEnterArrived, &cAIBehaviorMoveTo::OnUpdateArrived, &cAIBehaviorMoveTo::OnExitArrived)
 , m_moveGoal(point)
 , m_lastDistSqr(999999)
{
}

////////////////////////////////////////

cAIBehaviorMoveTo::~cAIBehaviorMoveTo()
{
}

////////////////////////////////////////

tResult cAIBehaviorMoveTo::Update(IAIAgent * pAgent, double elapsedTime)
{
   SafeRelease(m_pLocationProvider);
   SafeRelease(m_pAnimationProvider);
   if (pAgent != NULL)
   {
      pAgent->GetLocationProvider(&m_pLocationProvider);
      pAgent->GetAnimationProvider(&m_pAnimationProvider);
   }

   cStateMachine<cAIBehaviorMoveTo, double>::Update(elapsedTime);

   if (IsCurrentState(&m_arrivedState))
   {
      return S_AI_BEHAVIOR_DONE;
   }

   return S_AI_BEHAVIOR_CONTINUE;
}

////////////////////////////////////////

void cAIBehaviorMoveTo::OnInitialStateUpdate(double)
{
   // Go to the moving state immediately
   GotoState(&m_movingState);
}

////////////////////////////////////////

void cAIBehaviorMoveTo::OnEnterMoving()
{
   if (!!m_pAnimationProvider)
   {
      m_pAnimationProvider->SetAnimation(kAIAgentAnimWalk);
   }
}

////////////////////////////////////////

void cAIBehaviorMoveTo::OnUpdateMoving(double elapsed)
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

   float distSqr = Vec3DistanceSqr(curPos, m_moveGoal);

   if (AlmostEqual(curPos.x, m_moveGoal.x)
      && AlmostEqual(curPos.z, m_moveGoal.z))
   {
      GotoState(&m_arrivedState);
      // Force idle immediately
      if (!!m_pAnimationProvider)
      {
         m_pAnimationProvider->SetAnimation(kAIAgentAnimIdle);
      }
   }
   else
   {
      // If distance hasn't changed since last update, give up
      if (AlmostEqual(distSqr, m_lastDistSqr))
      {
         WarnMsg("Agent appears stuck... stopping move\n");
         GotoState(&m_arrivedState);
         return;
      }
      m_lastDistSqr = distSqr;

      tVec3 dir = m_moveGoal - curPos;
      dir.Normalize();
      curPos += (dir * 10.0f * static_cast<float>(elapsed));
      m_pLocationProvider->SetPosition(curPos);
   }
}

////////////////////////////////////////

void cAIBehaviorMoveTo::OnExitMoving()
{
}

////////////////////////////////////////

void cAIBehaviorMoveTo::OnEnterArrived()
{
   if (!!m_pAnimationProvider)
   {
      m_pAnimationProvider->SetAnimation(kAIAgentAnimIdle);
   }
}

////////////////////////////////////////

void cAIBehaviorMoveTo::OnUpdateArrived(double elapsed)
{
}

////////////////////////////////////////

void cAIBehaviorMoveTo::OnExitArrived()
{
}

////////////////////////////////////////

tResult AIBehaviorMoveToCreate(const tVec3 & point, IAIBehavior * * ppBehavior)
{
   if (ppBehavior == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IAIBehavior> pBehavior(static_cast<IAIBehavior *>(new cAIBehaviorMoveTo(point)));
   if (!pBehavior)
   {
      return E_OUTOFMEMORY;
   }
   return pBehavior.GetPointer(ppBehavior);
}


///////////////////////////////////////////////////////////////////////////////
