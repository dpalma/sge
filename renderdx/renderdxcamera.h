///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERDXCAMERA_H
#define INCLUDED_RENDERDXCAMERA_H

#include "render/renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderDXCamera
//

class cRenderDXCamera : public cComObject<IMPLEMENTS(IRenderCamera)>
{
   cRenderDXCamera(const cRenderDXCamera &);
   const cRenderDXCamera & operator =(const cRenderDXCamera &);

public:
   cRenderDXCamera();
   ~cRenderDXCamera();

   virtual const float * GetViewMatrix() const;
   virtual tResult GetViewMatrix(float viewMatrix[16]) const;
   virtual tResult SetViewMatrix(const float viewMatrix[16]);

   virtual const float * GetProjectionMatrix() const;
   virtual tResult GetProjectionMatrix(float projMatrix[16]) const;
   virtual tResult SetProjectionMatrix(const float projMatrix[16]);

   virtual tResult GetViewProjectionMatrix(float viewProjMatrix[16]) const;
   virtual tResult GetViewProjectionInverseMatrix(float viewProjInvMatrix[16]) const;

   virtual tResult ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy) const;
   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay) const;
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDERDXCAMERA_H
