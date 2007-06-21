///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERGLCAMERA_H
#define INCLUDED_RENDERGLCAMERA_H

#include "render/renderapi.h"

#include "tech/matrix4.h"

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

   virtual float GetFOV() const;
   virtual void SetFOV(float fov);

   virtual float GetAspect() const;
   virtual void SetAspect(float aspect);
   virtual void SetAutoAspect();

   virtual tResult GetNearFar(float * pZNear, float * pZFar) const;
   virtual tResult SetNearFar(float zNear, float zFar);

   void UpdateCompositeMatrices() const; // const because it must be called from const methods
   virtual tResult GetViewProjectionMatrix(float viewProjMatrix[16]) const;
   virtual tResult GetViewProjectionInverseMatrix(float viewProjInvMatrix[16]) const;

   virtual tResult ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy) const;
   virtual tResult GeneratePickRay(float ndx, float ndy, cRay<float> * pRay) const;

private:
   float m_view[16];

   float m_fov, m_aspect, m_znear, m_zfar;

   mutable tMatrix4 m_proj;

   enum eInternalFlags
   {
      kAutoAspect                   = (1 << 0),
      kUpdateCompositeMatrices      = (1 << 1),
      kUpdateProjectionMatrix       = (1 << 2),
   };

   mutable uint m_flags;

   mutable float m_viewInv[16], m_viewProj[16], m_viewProjInv[16];
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDERGLCAMERA_H
