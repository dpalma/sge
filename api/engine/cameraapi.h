///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CAMERAAPI_H
#define INCLUDED_CAMERAAPI_H

#include "enginedll.h"
#include "tech/comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

class cRay;
template <typename T> class cMatrix4;
typedef cMatrix4<float> tMatrix4;

F_DECLARE_INTERFACE(ICamera);
F_DECLARE_INTERFACE(ICameraControl);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ICamera
//

interface ICamera : IUnknown
{
   virtual void SetViewMatrix(const tMatrix4 & view) = 0;

   virtual void SetProjectionMatrix(const tMatrix4 & proj) = 0;

   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay) const = 0;

   virtual void SetGLState() = 0;
};

////////////////////////////////////////

ENGINE_API tResult CameraCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ICameraControl
//

enum eCameraMove
{
   kCameraMoveNone = 0,
   kCameraMoveLeft = 1,
   kCameraMoveRight = 2,
   kCameraMoveForward = 4,
   kCameraMoveBack = 8,
};

interface ICameraControl : IUnknown
{
   virtual tResult GetElevation(float * pElevation) const = 0;
   virtual tResult SetElevation(float elevation) = 0;

   virtual tResult GetPitch(float * pPitch) const = 0;
   virtual tResult SetPitch(float Pitch) = 0;

   // TODO: this function is only to support legacy code in game
   virtual tResult LookAtPoint(float x, float z) = 0;

   virtual void SetMovement(uint mask, uint flag) = 0;

   virtual tResult Raise() = 0;
   virtual tResult Lower() = 0;
};

////////////////////////////////////////

ENGINE_API tResult CameraControlCreate();


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CAMERAAPI_H
