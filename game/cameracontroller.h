///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CAMERACONTROLLER_H
#define INCLUDED_CAMERACONTROLLER_H

#include "comtools.h"
#include "sim.h"
#include "inputapi.h"
#include "matrix4.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(ISceneCamera);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGameCameraController
//

class cGameCameraController : public cComObject2<IMPLEMENTS(ISimClient), cDefaultInputListener, &IID_IInputListener>
{
   cGameCameraController(const cGameCameraController &);
   const cGameCameraController & operator =(const cGameCameraController &);

public:
   cGameCameraController(ISceneCamera * pCamera);
   ~cGameCameraController();

   void Connect();
   void Disconnect();

   virtual void OnFrame(double elapsedTime);

   virtual bool OnInputEvent(const sInputEvent * pEvent);

   void LookAtPoint(float x, float z);

private:
   float m_pitch, m_oneOverTangentPitch, m_elevation;
   tVec3 m_eye, m_focus, m_velocity;

   tMatrix4 m_rotation;

   cAutoIPtr<ISceneCamera> m_pCamera;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CAMERACONTROLLER_H
