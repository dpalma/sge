///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CAMERACONTROLLER_H
#define INCLUDED_CAMERACONTROLLER_H

#include "comtools.h"
#include "sim.h"
#include "input.h"
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

class cGameCameraController : public cComObject2<IMPLEMENTS(ISimClient), IMPLEMENTS(IInputListener)>
{
   cGameCameraController(const cGameCameraController &);
   const cGameCameraController & operator =(const cGameCameraController &);

public:
   cGameCameraController(ISceneCamera * pCamera);
   ~cGameCameraController();

   void Connect();
   void Disconnect();

   virtual void OnFrame(double elapsedTime);

   virtual bool OnMouseEvent(int x, int y, uint mouseState, double time);
   virtual bool OnKeyEvent(long key, bool down, double time);

   void LookAtPoint(float x, float z);

   const tVec3 & GetEyePosition() const { return m_eye; }

private:
   bool BuildPickRay(int x, int y, tVec3 * pRay);

   float m_pitch, m_oneOverTangentPitch, m_elevation;
   tVec3 m_eye, m_focus, m_velocity;

   tMatrix4 m_rotation;

   cAutoIPtr<ISceneCamera> m_pCamera;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CAMERACONTROLLER_H
