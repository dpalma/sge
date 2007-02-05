///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagentbehaviorwander.h"

#include "tech/multivar.h"
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

void cAIAgentBehaviorWander::MoveTo(const tVec3 & point)
{
   m_moveGoal = point;

   GotoState(&m_movingState);
}

////////////////////////////////////////

void cAIAgentBehaviorWander::OnEnterIdle()
{
   // TODO: send timer message to self
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
         // TODO: get random point on terrain
         //tVec3 point;
         //if ()
         //{
         //   cAutoIPtr<IAIAgentTask> pTask;
         //   if (AIAgentTaskMoveToCreate(point, &pTask) == S_OK)
         //   {
         //      amp.first->SetActiveTask(pTask);
         //   }
         //}
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
         GotoState(&m_idleState);
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
