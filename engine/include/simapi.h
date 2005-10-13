///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SIMAPI_H
#define INCLUDED_SIMAPI_H

#include "enginedll.h"
#include "comtools.h"
#include "connpt.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(ISim);
F_DECLARE_INTERFACE(ISimClient);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISim
//

interface ISim : IUnknown
{
   DECLARE_CONNECTION_POINT(ISimClient);

   virtual void Go() = 0;
   virtual void Stop() = 0;
   virtual void Reset() = 0;
   virtual bool IsRunning() const = 0;

   virtual double GetTime() const = 0;
   virtual double GetFrameTime() const = 0;

   virtual double GetTimeScale() const = 0;
   virtual void SetTimeScale(double scale) = 0;

   virtual void NextFrame() = 0;
};

///////////////////////////////////////

#define kSimObjName "Sim"

ENGINE_API tResult SimCreate();

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISimClient
//

interface ISimClient : IUnknown
{
   virtual void OnSimFrame(double elapsedTime) = 0;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SIMAPI_H
