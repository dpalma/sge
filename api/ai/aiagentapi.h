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
   kAIAMT_Stop,
   kAIAMT_MoveTo,
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

   virtual tResult RegisterAgent(tAIAgentID id, IAIAgent * pAgent) = 0;
   virtual tResult RevokeAgent(tAIAgentID id) = 0;
};

////////////////////////////////////////

AI_API tResult AIAgentMessageRouterCreate();


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgent
//

interface IAIAgent : IUnknown
{
   virtual tResult SetLocationProvider(IAIAgentLocationProvider * pLocationProvider) = 0;
   virtual tResult GetLocationProvider(IAIAgentLocationProvider * * ppLocationProvider) = 0;

   virtual tResult SetAnimationProvider(IAIAgentAnimationProvider * pAnimationProvider) = 0;
   virtual tResult GetAnimationProvider(IAIAgentAnimationProvider * * ppAnimationProvider) = 0;

   virtual tResult Update(double time) = 0;

   virtual tResult HandleMessage(IAIAgentMessage * pMessage) = 0;
};

////////////////////////////////////////

AI_API tResult AIAgentCreate(tAIAgentID id, IUnknown * pUnkOuter, IAIAgent * * ppAgent);


////////////////////////////////////////////////////////////////////////////////
//
// ENUM: eAIAgentAnimation
//

enum eAIAgentAnimation
{
   kAIAgentAnimIdle,
   kAIAgentAnimWalk,
   kAIAgentAnimRun,
   kAIAgentAnimMeleeAttack,
   kAIAgentAnimRangedAttack,
   kAIAgentAnimDie,
   kAIAgentAnimTakeDamage,
};


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgentAnimationProvider
//

interface IAIAgentAnimationProvider : IUnknown
{
   virtual tResult SetAnimation(eAIAgentAnimation anim) = 0;
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
// INTERFACE: IAIAgentTask
//

////////////////////////////////////////

enum eAIAgentTaskResult
{
   S_AI_AGENT_TASK_ERROR      = E_FAIL,
   S_AI_AGENT_TASK_CONTINUE   = S_OK,
   S_AI_AGENT_TASK_DONE       = S_FALSE,
};

////////////////////////////////////////

interface IAIAgentTask : IUnknown
{
   virtual tResult Update(IAIAgent * pAgent, double time) = 0;
};

////////////////////////////////////////

AI_API tResult AIAgentTaskStandCreate(IAIAgentTask * * ppTask);
AI_API tResult AIAgentTaskMoveToCreate(const tVec3 & point, IAIAgentTask * * ppTask);


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAGENTAPI_H
