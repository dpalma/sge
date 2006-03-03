///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CAMERA_H
#define INCLUDED_CAMERA_H

#include "cameraapi.h"
#include "inputapi.h"
#include "simapi.h"
#include "frustum.h"
#include "globalobjdef.h"
#include "matrix4.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCamera
//

class cCamera : public cComObject2<IMPLEMENTS(ICamera), IMPLEMENTS(IGlobalObject)>
{
public:
   cCamera();
   ~cCamera();

   DECLARE_NAME(Camera)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual void SetPerspective(float fov, float aspect, float znear, float zfar);
   virtual void SetOrtho(float left, float right, float bottom, float top, float znear, float zfar);

   virtual const tMatrix4 & GetViewMatrix() const;
   virtual void SetViewMatrix(const tMatrix4 & view);

   virtual const tMatrix4 & GetProjectionMatrix() const;
   virtual void SetProjectionMatrix(const tMatrix4 & proj);

   virtual const tMatrix4 & GetViewProjectionMatrix() const;
   virtual const tMatrix4 & GetViewProjectionInverseMatrix() const;

   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay) const;

   virtual void SetGLState();

private:
   void UpdateCompositeMatrices() const; // const because it's called from const accessors

   tMatrix4 m_proj, m_view;

   mutable bool m_bUpdateCompositeMatrices;
   mutable tMatrix4 m_viewInv, m_viewProj, m_viewProjInv;
   mutable cFrustum m_frustum;
};

///////////////////////////////////////

inline const tMatrix4 & cCamera::GetViewMatrix() const
{
   return m_view;
}

///////////////////////////////////////

inline void cCamera::SetViewMatrix(const tMatrix4 & view)
{
   m_view = view;
   m_bUpdateCompositeMatrices = true;
}

///////////////////////////////////////

inline const tMatrix4 & cCamera::GetProjectionMatrix() const
{
   return m_proj;
}

///////////////////////////////////////

inline void cCamera::SetProjectionMatrix(const tMatrix4 & proj)
{
   m_proj = proj;
   m_bUpdateCompositeMatrices = true;
}

///////////////////////////////////////

inline const tMatrix4 & cCamera::GetViewProjectionMatrix() const
{
   UpdateCompositeMatrices();
   return m_viewProj;
}

///////////////////////////////////////

inline const tMatrix4 & cCamera::GetViewProjectionInverseMatrix() const
{
   UpdateCompositeMatrices();
   return m_viewProjInv;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCameraControl
//

class cCameraControl : public cComObject4<IMPLEMENTS(ICameraControl),
                                          IMPLEMENTS(ISimClient),
                                          IMPLEMENTS(IInputListener),
                                          IMPLEMENTS(IGlobalObject)>
{
public:
   cCameraControl();
   ~cCameraControl();

   DECLARE_NAME(CameraControl)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual bool OnInputEvent(const sInputEvent * pEvent);

   virtual void OnSimFrame(double elapsedTime);

   virtual tResult LookAtPoint(float x, float z);
   virtual tResult MoveLeft();
   virtual tResult MoveRight();
   virtual tResult MoveForward();
   virtual tResult MoveBack();
   virtual tResult Raise();
   virtual tResult Lower();

private:
   float m_pitch, m_oneOverTangentPitch, m_elevation;
   tVec3 m_eye, m_focus, m_velocity;
   tMatrix4 m_rotation;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CAMERA_H
