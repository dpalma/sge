///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "aiagentbehaviorfolloworders.h"

#include "tech/multivar.h"

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
// CLASS: cAIAgentBehaviorFollowOrders
//

////////////////////////////////////////

cAIAgentBehaviorFollowOrders::cAIAgentBehaviorFollowOrders()
{
}

////////////////////////////////////////

cAIAgentBehaviorFollowOrders::~cAIAgentBehaviorFollowOrders()
{
}

////////////////////////////////////////

tResult cAIAgentBehaviorFollowOrders::HandleMessage(IAIAgent * pAgent, IAIAgentMessage * pMessage)
{
   Assert(pAgent != NULL);
   Assert(pMessage != NULL);

   cAutoIPtr<IAIAgent> pStabilizeAgent(CTAddRef(pAgent));
   cAutoIPtr<IAIAgentMessage> pStabilizeMessage(CTAddRef(pMessage));

   eAIAgentMessageType msgType = pMessage->GetMessageType();

   switch (msgType)
   {
      case kAIAMT_OrderStop:
      {
         pAgent->SetActiveTask(NULL);
         break;
      }

      case kAIAMT_OrderMoveTo:
      {
         tVec3 point;
         if (GetVec3(pMessage, &point))
         {
            cAutoIPtr<IAIAgentTask> pTask;
            if (AIAgentTaskMoveToCreate(point, &pTask) == S_OK)
            {
               pAgent->SetActiveTask(pTask);
            }
         }
         break;
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult AIAgentBehaviorFollowOrdersCreate(IAIAgentBehavior * * ppBehavior)
{
   if (ppBehavior == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IAIAgentBehavior> pBehavior(static_cast<IAIAgentBehavior *>(new cAIAgentBehaviorFollowOrders));
   if (!pBehavior)
   {
      return E_OUTOFMEMORY;
   }
   return pBehavior.GetPointer(ppBehavior);
}

///////////////////////////////////////////////////////////////////////////////
