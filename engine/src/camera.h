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

template <typename T>
T Lerp(const T & a, const T & b, float t)
{
   return a + (t * (b - a));
}


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cTimedLerp
//

template <typename T>
class cTimedLerp
{
public:
   cTimedLerp()
    : m_start(0)
    , m_end(0)
    , m_t(0)
    , m_oneOverTimeSpan(0)
   {
   }

   cTimedLerp(const T & start, const T & end, const T & rate)
    : m_start(start)
    , m_end(end)
    , m_t(0)
    , m_oneOverTimeSpan(fabs(static_cast<double>(rate) / (end - start)))
   {
   }

   cTimedLerp(const cTimedLerp & other)
    : m_start(other.m_start)
    , m_end(other.m_end)
    , m_t(other.m_t)
    , m_oneOverTimeSpan(other.m_oneOverTimeSpan)
   {
   }

   ~cTimedLerp()
   {
   }

   const cTimedLerp & operator =(const cTimedLerp & other)
   {
      m_start = other.m_start;
      m_end = other.m_end;
      m_t = other.m_t;
      m_oneOverTimeSpan = other.m_oneOverTimeSpan;
      return *this;
   }

   void Restart(const T & start, const T & end, const T & rate)
   {
      m_start = start;
      m_end = end;
      m_t = 0;
      m_oneOverTimeSpan = fabs(static_cast<double>(rate) / (end - start));
   }

   T Update(double time)
   {
      if (m_start == m_end)
      {
         return m_end;
      }
      m_t += (time * m_oneOverTimeSpan);
      if (m_t >= 1)
      {
         m_t = 1;
         return m_end;
      }
      return Lerp<T>(m_start, m_end, static_cast<float>(m_t));
   }

private:
   T m_start, m_end;
   double m_t, m_oneOverTimeSpan;
};


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

   virtual tResult GetElevation(float * pElevation) const;
   virtual tResult SetElevation(float elevation);
   virtual tResult GetPitch(float * pPitch) const;
   virtual tResult SetPitch(float Pitch);

   virtual tResult LookAtPoint(float x, float z);
   virtual tResult MoveLeft();
   virtual tResult MoveRight();
   virtual tResult MoveForward();
   virtual tResult MoveBack();
   virtual tResult Raise();
   virtual tResult Lower();

private:
   float m_pitch, m_oneOverTangentPitch, m_elevation;
   tVec3 m_eye, m_focus;
   tMatrix4 m_rotation;
   cTimedLerp<float> m_elevationLerp;
   cTimedLerp<float> m_leftRightLerp;
   cTimedLerp<float> m_forwardBackLerp;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CAMERA_H
