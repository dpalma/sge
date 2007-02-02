///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagenttaskmoveto.h"

#include "tech/statemachinetem.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentTaskMoveTo
//

////////////////////////////////////////

cAIAgentTaskMoveTo::cAIAgentTaskMoveTo(const tVec3 & point)
 : m_movingState(&cAIAgentTaskMoveTo::OnEnterMoving, &cAIAgentTaskMoveTo::OnUpdateMoving, &cAIAgentTaskMoveTo::OnExitMoving)
 , m_arrivedState(&cAIAgentTaskMoveTo::OnEnterArrived, &cAIAgentTaskMoveTo::OnUpdateArrived, &cAIAgentTaskMoveTo::OnExitArrived)
 , m_moveGoal(point)
 , m_lastDistSqr(999999)
{
}

////////////////////////////////////////

cAIAgentTaskMoveTo::~cAIAgentTaskMoveTo()
{
}

////////////////////////////////////////

tResult cAIAgentTaskMoveTo::Update(IAIAgent * pAgent, double elapsedTime)
{
   SafeRelease(m_pLocationProvider);
   SafeRelease(m_pAnimationProvider);
   if (pAgent != NULL)
   {
      pAgent->GetLocationProvider(&m_pLocationProvider);
      pAgent->GetAnimationProvider(&m_pAnimationProvider);
   }

   cStateMachine<cAIAgentTaskMoveTo, double>::Update(elapsedTime);

   if (IsCurrentState(&m_arrivedState))
   {
      return S_AI_AGENT_TASK_DONE;
   }

   return S_AI_AGENT_TASK_CONTINUE;
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnInitialStateUpdate(double)
{
   // Go to the moving state immediately
   GotoState(&m_movingState);
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnEnterMoving()
{
   if (!!m_pAnimationProvider)
   {
      m_pAnimationProvider->RequestAnimation(kAIAA_Walk);
   }
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnUpdateMoving(double elapsed)
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
         m_pAnimationProvider->RequestAnimation(kAIAA_Fidget);
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

void cAIAgentTaskMoveTo::OnExitMoving()
{
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnEnterArrived()
{
   if (!!m_pAnimationProvider)
   {
      m_pAnimationProvider->RequestAnimation(kAIAA_Fidget);
   }
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnUpdateArrived(double elapsed)
{
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnExitArrived()
{
}

////////////////////////////////////////

tResult AIAgentTaskMoveToCreate(const tVec3 & point, IAIAgentTask * * ppTask)
{
   if (ppTask == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IAIAgentTask> pTask(static_cast<IAIAgentTask *>(new cAIAgentTaskMoveTo(point)));
   if (!pTask)
   {
      return E_OUTOFMEMORY;
   }
   return pTask.GetPointer(ppTask);
}


///////////////////////////////////////////////////////////////////////////////
