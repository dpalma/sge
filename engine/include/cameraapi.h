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

#endif // !INCLUDED_CAMERAAPI_H
