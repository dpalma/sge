///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CAMERA_H
#define INCLUDED_CAMERA_H

#include "engine/cameraapi.h"
#include "engine/saveloadapi.h"
#include "tech/schedulerapi.h"

#include "tech/globalobjdef.h"
#include "tech/matrix4.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_GUID(SAVELOADID_CameraControl);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCameraControl
//

class cCameraControl : public cComObject3<IMPLEMENTS(ICameraControl),
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

   void SimFrame(double elapsedTime);

   virtual tResult GetElevation(float * pElevation) const;
   virtual tResult SetElevation(float elevation);
   virtual tResult GetPitch(float * pPitch) const;
   virtual tResult SetPitch(float Pitch);

   virtual tResult LookAtPoint(float x, float z);
   virtual void SetMovement(uint mask, uint flag);
   virtual tResult Raise();
   virtual tResult Lower();

   virtual bool HandleInputEvent(const sInputEvent *);

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
