///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CAMERAAPI_H
#define INCLUDED_CAMERAAPI_H

#include "enginedll.h"
#include "comtools.h"

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
   virtual void SetPerspective(float fov, float aspect, float znear, float zfar) = 0;
   virtual void SetOrtho(float left, float right, float bottom, float top, float znear, float zfar) = 0;

   virtual const tMatrix4 & GetViewMatrix() const = 0;
   virtual void SetViewMatrix(const tMatrix4 & view) = 0;

   virtual const tMatrix4 & GetProjectionMatrix() const = 0;
   virtual void SetProjectionMatrix(const tMatrix4 & proj) = 0;

   virtual const tMatrix4 & GetViewProjectionMatrix() const = 0;
   virtual const tMatrix4 & GetViewProjectionInverseMatrix() const = 0;

   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay) const = 0;

   virtual void SetGLState() = 0;
};

////////////////////////////////////////

ENGINE_API tResult CameraCreate();

///////////////////////////////////////////////////////////////////////////////

ENGINE_API tResult ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ICameraControl
//

interface ICameraControl : IUnknown
{
   // TODO: this function is only to support legacy code in game
   virtual tResult LookAtPoint(float x, float z) = 0;

   virtual tResult MoveLeft() = 0;
   virtual tResult MoveRight() = 0;
   virtual tResult MoveForward() = 0;
   virtual tResult MoveBack() = 0;
   virtual tResult Raise() = 0;
   virtual tResult Lower() = 0;
};

////////////////////////////////////////

ENGINE_API tResult CameraControlCreate();


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CAMERAAPI_H
