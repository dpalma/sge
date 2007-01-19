////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_AIAPI_H
#define INCLUDED_AIAPI_H

#include "aidll.h"

#include "tech/comtools.h"
#include "tech/quat.h"
#include "tech/vec3.h"

#pragma once

F_DECLARE_INTERFACE_GUID(IAIAgent, "FD56C9A7-ABED-4fab-BC3E-DDED0BFBE3CD");
F_DECLARE_INTERFACE_GUID(IAIAgentAnimationProvider, "664659A6-82CF-4ed3-8602-40A056E417F9");
F_DECLARE_INTERFACE_GUID(IAIAgentLocationProvider, "38DB75CF-1DC3-438f-A1C9-A4106E29DDB2");
F_DECLARE_INTERFACE_GUID(IAIBehavior, "A80E9587-0037-46bd-BCBA-B97D6E6E15C4");


////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IAIAgent
//

interface IAIAgent : IUnknown
{
   virtual tResult SetDefaultBehavior(IAIBehavior * pBehavior) = 0;
   virtual tResult GetDefaultBehavior(IAIBehavior * * ppBehavior) = 0;

   virtual tResult PushBehavior(IAIBehavior * pBehavior) = 0;
   virtual tResult PopBehavior() = 0;

   virtual tResult GetActiveBehavior(IAIBehavior * * ppBehavior) = 0;

   virtual tResult SetLocationProvider(IAIAgentLocationProvider * pLocationProvider) = 0;
   virtual tResult GetLocationProvider(IAIAgentLocationProvider * * ppLocationProvider) = 0;

   virtual tResult SetAnimationProvider(IAIAgentAnimationProvider * pAnimationProvider) = 0;
   virtual tResult GetAnimationProvider(IAIAgentAnimationProvider * * ppAnimationProvider) = 0;
};

////////////////////////////////////////

AI_API tResult AIAgentCreate(IAIAgent * * ppAgent);


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
// INTERFACE: IAIBehavior
//

////////////////////////////////////////

enum eAIBehaviorResult
{
   S_AI_BEHAVIOR_ERROR        = E_FAIL,
   S_AI_BEHAVIOR_CONTINUE     = S_OK,
   S_AI_BEHAVIOR_DONE         = S_FALSE,
};

////////////////////////////////////////

interface IAIBehavior : IUnknown
{
   virtual tResult Update(IAIAgent * pAgent, double elapsedTime) = 0;
};

////////////////////////////////////////

AI_API tResult AIBehaviorStandCreate(IAIBehavior * * ppBehavior);
AI_API tResult AIBehaviorMoveToCreate(const tVec3 & point, IAIBehavior * * ppBehavior);


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_AIAPI_H
