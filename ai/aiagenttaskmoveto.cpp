///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagenttaskmoveto.h"

#include "tech/quat.inl"
#include "tech/statemachinetem.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentTaskMoveTo
//

////////////////////////////////////////

cAIAgentTaskMoveTo::cAIAgentTaskMoveTo(const float point[3])
 : tStateMachine(&m_movingState)
 , m_movingState(&cAIAgentTaskMoveTo::OnEnterMoving,
                 &cAIAgentTaskMoveTo::OnExitMoving,
                 &cAIAgentTaskMoveTo::OnUpdateMoving)
 , m_arrivedState(&cAIAgentTaskMoveTo::OnEnterArrived,
                  &cAIAgentTaskMoveTo::OnExitArrived,
                  &cAIAgentTaskMoveTo::OnUpdateArrived)
 , m_moveGoal((point != NULL) ? point : cPoint3<float>(0,0,0))
 , m_lastDistSqr(999999)
 , m_bJustStarted(true)
{
}

////////////////////////////////////////

cAIAgentTaskMoveTo::~cAIAgentTaskMoveTo()
{
}

////////////////////////////////////////

tResult cAIAgentTaskMoveTo::Update(IAIAgent * pAgent, double time)
{
   tStateMachine::Update(std::make_pair(pAgent, time));

   if (IsCurrentState(&m_arrivedState))
   {
      return S_AI_DONE;
   }

   return S_AI_CONTINUE;
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnEnterMoving()
{
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnExitMoving()
{
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnUpdateMoving(const tAgentDoublePair & adp)
{
   cAutoIPtr<IAIAgentAnimationProvider> pAnimationProvider;
   cAutoIPtr<IAIAgentLocationProvider> pLocationProvider;
   if (adp.first->GetAnimationProvider(&pAnimationProvider) != S_OK
      || adp.first->GetLocationProvider(&pLocationProvider) != S_OK)
   {
      return;
   }

   if (m_bJustStarted)
   {
      pAnimationProvider->RequestAnimation(kAIAA_Walk);
      m_bJustStarted = false;
   }

   cPoint3<float> curPos;
   if (pLocationProvider->GetPosition(curPos.xyz) != S_OK)
   {
      return;
   }

   float distSqr = DistanceSqr(curPos, m_moveGoal);

   if (AlmostEqual(curPos.x, m_moveGoal.x)
      && AlmostEqual(curPos.z, m_moveGoal.z))
   {
      GotoState(&m_arrivedState);
      // Force idle immediately
      pAnimationProvider->RequestAnimation(kAIAA_Fidget);
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

      tVec3 dir(m_moveGoal.x - curPos.x, m_moveGoal.y - curPos.y, m_moveGoal.z - curPos.z);
      dir.Normalize();

      tVec3 scaledDir(dir * 10.0f * static_cast<float>(adp.second)); // second member of pair is elapsed time

      curPos.x += scaledDir.x;
      curPos.y += scaledDir.y;
      curPos.z += scaledDir.z;

      pLocationProvider->SetPosition(curPos.xyz);

      static const tVec3 axis(0,0,1);

      tVec3::value_type d = axis.Dot(dir);
      pLocationProvider->SetOrientation(tQuat(0,1,0,acos(d)));
   }
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnEnterArrived()
{
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnExitArrived()
{
}

////////////////////////////////////////

void cAIAgentTaskMoveTo::OnUpdateArrived(const tAgentDoublePair &)
{
}

////////////////////////////////////////

tResult AIAgentTaskMoveToCreate(const float point[3], IAIAgentTask * * ppTask)
{
   if (point == NULL || ppTask == NULL)
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
