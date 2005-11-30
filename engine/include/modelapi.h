///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELAPI_H
#define INCLUDED_MODELAPI_H

#include "enginedll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

enum eModelAnimationType
{
   kMAT_Walk,
   kMAT_Run,
   kMAT_Death,
   kMAT_Attack,
   kMAT_Damage,
   kMAT_Idle,
};

struct sModelAnimationDesc
{
   eModelAnimationType type;
   uint start, end, fps;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELAPI_H
