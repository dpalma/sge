////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAGENTAPI_H
#define INCLUDED_AIAGENTAPI_H

#include "aidll.h"

#include "tech/comtools.h"
#include "tech/quat.h"
#include "tech/vec3.h"

#pragma once

F_DECLARE_INTERFACE_GUID(IAIAgent, "FD56C9A7-ABED-4fab-BC3E-DDED0BFBE3CD");
F_DECLARE_INTERFACE_GUID(IAIAgentAnimationProvider, "664659A6-82CF-4ed3-8602-40A056E417F9");
F_DECLARE_INTERFACE_GUID(IAIAgentLocationProvider, "38DB75CF-1DC3-438f-A1C9-A4106E29DDB2");
F_DECLARE_INTERFACE_GUID(IAIAgentMessage, "6B5E445D-913A-4d47-B003-B41C7AC1D0AC");
F_DECLARE_INTERFACE_GUID(IAIAgentMessageRouter, "7835EFCF-D50A-461d-BB4F-683C08E75C7F");
F_DECLARE_INTERFACE_GUID(IAIAgentBehavior, "FB6CD4BF-A388-4045-A7DD-DB1DC294953E");
F_DECLARE_INTERFACE_GUID(IAIAgentTask, "A80E9587-0037-46bd-BCBA-B97D6E6E15C4");

class cMultiVar;


////////////////////////////////////////////////////////////////////////////////

typedef ulong tAIAgentID;

const tAIAgentID kInvalidAIAgentID = 0;


////////////////////////////////////////////////////////////////////////////////
//
// ENUM: eAIAgentMessageType
//

enum eAIAgentMessageType
{
   kAIAMT_Stop,               // no arguments
   kAIAMT_MoveTo,             // three floats indicating the goal position (x,y,z)
   kAIAMT_TimeOut,            // optional integer for use as a timer identifier
   kAIAMT_TaskDone,           // IAIAgentTask interface pointer
   kAIAMT_BehaviorBegin,      // IAIAgentBehavior interface pointer
   kAIAMT_BehaviorEnd,        // IAIAgentBehavior interface pointer
   kAIAMT_BehaviorPause,      // IAIAgentBehavior interface pointer
   kAIAMT_BehaviorResume,     // IAIAgentBehavior interface pointer
};


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgentMessage
//

interface IAIAgentMessage : IUnknown
{
   virtual tAIAgentID GetReceiver() const = 0;
   virtual tAIAgentID GetSender() const = 0;

   virtual double GetDeliveryTime() const = 0;

   virtual eAIAgentMessageType GetMessageType() const = 0;

   virtual uint GetArgumentCount() const = 0;
   virtual tResult GetArgument(uint index, cMultiVar * pArg) const = 0;
   virtual tResult GetArguments(uint * pnArgs, cMultiVar * pArgs) const = 0;
};


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgentMessageRouter
//

interface IAIAgentMessageRouter : IUnknown
{
   virtual tResult SendMessage(tAIAgentID receiver, tAIAgentID sender,
      eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args) = 0;

   virtual tResult SendMessage(tAIAgentID receiver, tAIAgentID sender, double deliveryTime,
      eAIAgentMessageType messageType, uint nArgs, const cMultiVar * args) = 0;

   virtual tResult RegisterAgent(IAIAgent * pAgent) = 0;
   virtual tResult RevokeAgent(IAIAgent * pAgent) = 0;
};

////////////////////////////////////////

AI_API tResult AIAgentMessageRouterCreate();


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgent
//

interface IAIAgent : IUnknown
{
   virtual tAIAgentID GetID() const = 0;

   virtual tResult SetLocationProvider(IAIAgentLocationProvider * pLocationProvider) = 0;
   virtual tResult GetLocationProvider(IAIAgentLocationProvider * * ppLocationProvider) = 0;

   virtual tResult SetAnimationProvider(IAIAgentAnimationProvider * pAnimationProvider) = 0;
   virtual tResult GetAnimationProvider(IAIAgentAnimationProvider * * ppAnimationProvider) = 0;

   virtual tResult PushBehavior(IAIAgentBehavior * pBehavior) = 0;
   virtual tResult PopBehavior() = 0;

   virtual tResult SetActiveTask(IAIAgentTask * pTask) = 0;
   virtual tResult GetActiveTask(IAIAgentTask * * ppTask) = 0;

   virtual tResult Update(double time) = 0;

   virtual tResult HandleMessage(IAIAgentMessage * pMessage) = 0;
};

////////////////////////////////////////

AI_API tResult AIAgentCreate(tAIAgentID id, IUnknown * pUnkOuter, IAIAgent * * ppAgent);


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgentAnimationProvider
//

////////////////////////////////////////

enum eAIAgentAnimation
{
   kAIAA_Stand,
   kAIAA_Fidget,
   kAIAA_Walk,
   kAIAA_Run,
   kAIAA_MeleeAttack,
   kAIAA_RangedAttack,
   kAIAA_Die,
   kAIAA_TakeDamage,
};

////////////////////////////////////////

interface IAIAgentAnimationProvider : IUnknown
{
   virtual tResult RequestAnimation(eAIAgentAnimation anim) = 0;
};


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgentLocationProvider
//

interface IAIAgentLocationProvider : IUnknown
{
   virtual tResult SetPosition(const tVec3 & position) = 0;
   virtual tResult GetPosition(tVec3 * pPosition) const = 0;
};


////////////////////////////////////////////////////////////////////////////////
//
// ENUM: eAIResult
//

enum eAIResult
{
   E_AI_ERROR        = E_FAIL,
   S_AI_CONTINUE     = S_OK,
   S_AI_DONE         = S_FALSE,
};


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgentBehavior
//

interface IAIAgentBehavior : IUnknown
{
   virtual tResult HandleMessage(IAIAgent * pAgent, IAIAgentMessage * pMessage) = 0;
};

////////////////////////////////////////

AI_API tResult AIAgentBehaviorWanderCreate(IAIAgentBehavior * * ppBehavior);


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgentTask
//

interface IAIAgentTask : IUnknown
{
   virtual tResult Update(IAIAgent * pAgent, double time) = 0;
};

////////////////////////////////////////

AI_API tResult AIAgentTaskStandCreate(IAIAgentTask * * ppTask);
AI_API tResult AIAgentTaskMoveToCreate(const tVec3 & point, IAIAgentTask * * ppTask);


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENTAPI_H
