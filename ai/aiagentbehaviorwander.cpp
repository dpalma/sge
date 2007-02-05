///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagentbehaviorwander.h"

#include "engine/scenarioapi.h"
#include "engine/terrainapi.h"

#include "tech/globalobj.h"
#include "tech/multivar.h"
#include "tech/simapi.h"
#include "tech/statemachinetem.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

static bool GetVec3(IAIAgentMessage * pMessage, tVec3 * pVec)
{
   if (pMessage->GetArgumentCount() == 3)
   {
      cMultiVar args[3];
      for (int i = 0; i < _countof(args); ++i)
      {
         if (pMessage->GetArgument(i, &args[i]) != S_OK || !IsNumber(args[i]))
         {
            return false;
         }
      }

      *pVec = tVec3(args[0].ToFloat(), args[1].ToFloat(), args[2].ToFloat());
      return true;
   }

   return false;
}


///////////////////////////////////////////////////////////////////////////////

static bool GetWanderPoint(tVec3 * pVec)
{
   UseGlobal(Scenario);
   UseGlobal(TerrainModel);

   if (!pScenario || !pTerrainModel)
   {
      return false;
   }

   float nx = pScenario->NextRandFloat(), nz = pScenario->NextRandFloat();

   Assert(nx >= 0 && nx <= 1);
   Assert(nz >= 0 && nz <= 1);

   return (pTerrainModel->GetPointOnTerrain(nx, nz, pVec) == S_OK);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAIAgentBehaviorWander
//

////////////////////////////////////////

cAIAgentBehaviorWander::cAIAgentBehaviorWander()
 : tStateMachine(&m_idleState)
 , m_idleState(&cAIAgentBehaviorWander::OnEnterIdle,
               &cAIAgentBehaviorWander::OnExitIdle,
               &cAIAgentBehaviorWander::OnHandleMessageIdle)
 , m_movingState(&cAIAgentBehaviorWander::OnEnterMoving,
                 &cAIAgentBehaviorWander::OnExitMoving,
                 &cAIAgentBehaviorWander::OnHandleMessageMoving)
{
}

////////////////////////////////////////

cAIAgentBehaviorWander::~cAIAgentBehaviorWander()
{
}

////////////////////////////////////////

tResult cAIAgentBehaviorWander::HandleMessage(IAIAgent * pAgent, IAIAgentMessage * pMessage)
{
   Assert(pAgent != NULL);
   Assert(pMessage != NULL);

   cAutoIPtr<IAIAgent> pStabilizeAgent(CTAddRef(pAgent));
   cAutoIPtr<IAIAgentMessage> pStabilizeMessage(CTAddRef(pMessage));

   tAgentMessagePair agentMessagePair = std::make_pair(pAgent, pMessage);

   tStateMachine::Update(agentMessagePair);

   return S_OK;
}

////////////////////////////////////////

void cAIAgentBehaviorWander::BeginIdle(IAIAgent * pAgent)
{
   UseGlobal(Sim);
   UseGlobal(AIAgentMessageRouter);
   pAIAgentMessageRouter->SendMessage(pAgent->GetID(), pAgent->GetID(), pSim->GetTime() + 1, kAIAMT_TimeOut, 0, NULL);
   GotoState(&m_idleState);
}

////////////////////////////////////////

void cAIAgentBehaviorWander::OnEnterIdle()
{
}

////////////////////////////////////////

void cAIAgentBehaviorWander::OnExitIdle()
{
}

////////////////////////////////////////

void cAIAgentBehaviorWander::OnHandleMessageIdle(const tAgentMessagePair & amp)
{
   eAIAgentMessageType msgType = amp.second->GetMessageType();

   switch (msgType)
   {
      case kAIAMT_TimeOut:
      {
         tVec3 point;
         if (GetWanderPoint(&point))
         {
            cAutoIPtr<IAIAgentTask> pTask;
            if (AIAgentTaskMoveToCreate(point, &pTask) == S_OK)
            {
               amp.first->SetActiveTask(pTask);
            }
            GotoState(&m_movingState);
         }
         break;
      }

      case kAIAMT_BehaviorBegin:
      {
         BeginIdle(amp.first);
         break;
      }
   }
}

////////////////////////////////////////

void cAIAgentBehaviorWander::OnEnterMoving()
{
}

////////////////////////////////////////

void cAIAgentBehaviorWander::OnExitMoving()
{
}

////////////////////////////////////////

void cAIAgentBehaviorWander::OnHandleMessageMoving(const tAgentMessagePair & amp)
{
   eAIAgentMessageType msgType = amp.second->GetMessageType();

   switch (msgType)
   {
      case kAIAMT_TaskDone:
      {
         BeginIdle(amp.first);
         break;
      }
   }
}

////////////////////////////////////////

tResult AIAgentBehaviorWanderCreate(IAIAgentBehavior * * ppBehavior)
{
   if (ppBehavior == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IAIAgentBehavior> pBehavior(static_cast<IAIAgentBehavior *>(new cAIAgentBehaviorWander));
   if (!pBehavior)
   {
      return E_OUTOFMEMORY;
   }
   return pBehavior.GetPointer(ppBehavior);
}

///////////////////////////////////////////////////////////////////////////////
