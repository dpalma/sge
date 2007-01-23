///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERGLCAMERA_H
#define INCLUDED_RENDERGLCAMERA_H

#include "render/renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderGLCamera
//

class cRenderGLCamera : public cComObject<IMPLEMENTS(IRenderCamera)>
{
   cRenderGLCamera(const cRenderGLCamera &);
   const cRenderGLCamera & operator =(const cRenderGLCamera &);

public:
   cRenderGLCamera();
   ~cRenderGLCamera();

   virtual const float * GetViewMatrix() const;
   virtual tResult GetViewMatrix(float viewMatrix[16]) const;
   virtual tResult SetViewMatrix(const float viewMatrix[16]);

   virtual const float * GetProjectionMatrix() const;
   virtual tResult GetProjectionMatrix(float projMatrix[16]) const;
   virtual tResult SetProjectionMatrix(const float projMatrix[16]);

   void UpdateCompositeMatrices() const; // const because it must be called from const methods
   virtual tResult GetViewProjectionMatrix(float viewProjMatrix[16]) const;
   virtual tResult GetViewProjectionInverseMatrix(float viewProjInvMatrix[16]) const;

   virtual tResult ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy) const;
   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay) const;

private:
   float m_view[16], m_proj[16];

   mutable bool m_bUpdateCompositeMatrices;
   mutable float m_viewInv[16], m_viewProj[16], m_viewProjInv[16];
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDERGLCAMERA_H
