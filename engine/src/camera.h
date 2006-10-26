///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CAMERA_H
#define INCLUDED_CAMERA_H

#include "engine/cameraapi.h"
#include "platform/inputapi.h"
#include "engine/saveloadapi.h"
#include "tech/schedulerapi.h"

#include "tech/frustum.h"
#include "tech/globalobjdef.h"
#include "tech/matrix4.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_GUID(SAVELOADID_CameraControl);

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
    , m_oneOverTimeSpan((start != end) ? fabs(static_cast<double>(rate) / (end - start)) : 1)
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
      if (start != end)
      {
         m_oneOverTimeSpan = fabs(static_cast<double>(rate) / (end - start));
      }
      else
      {
         m_oneOverTimeSpan = 1;
      }
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
                                          IMPLEMENTS(IInputListener),
                                          IMPLEMENTS(IGlobalObject),
                                          IMPLEMENTS(ISaveLoadParticipant)>
{
   static const int gm_currentSaveLoadVer;

public:
   cCameraControl();
   ~cCameraControl();

   DECLARE_NAME(CameraControl)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual bool OnInputEvent(const sInputEvent * pEvent);

   void SimFrame(double elapsedTime);

   virtual tResult GetElevation(float * pElevation) const;
   virtual tResult SetElevation(float elevation);
   virtual tResult GetPitch(float * pPitch) const;
   virtual tResult SetPitch(float Pitch);

   virtual tResult LookAtPoint(float x, float z);
   virtual void SetMovement(uint mask, uint flag);
   virtual tResult Raise();
   virtual tResult Lower();

   // ISaveLoadParticipant methods
   virtual tResult Save(IWriter *);
   virtual tResult Load(IReader *, int version);
   virtual void Reset();

private:
   class cMoveCameraTask : public cComObject<IMPLEMENTS(ITask)>
   {
   public:
      cMoveCameraTask(cCameraControl * pOuter);
      virtual void DeleteThis() {}
      virtual tResult Execute(double time);
   private:
      cCameraControl * m_pOuter;
      double m_lastTime;
   };
   friend class cMoveCameraTask;
   cMoveCameraTask m_moveCameraTask;

   uint m_cameraMove;
   float m_pitch, m_oneOverTangentPitch, m_elevation;
   tVec3 m_eye, m_focus;
   tMatrix4 m_rotation;
   cTimedLerp<float> m_elevationLerp;
   bool m_bPitchChanged;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CAMERA_H
