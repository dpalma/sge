///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "cameracontroller.h"

#include "cameraapi.h"
#include "inputapi.h"
#include "ray.h"

#include "configapi.h"
#include "globalobj.h"
#include "keys.h"

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

static const float kDefaultElevation = 100;
static const float kDefaultPitch = 70;
static const float kDefaultSpeed = 50;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGameCameraController
//

///////////////////////////////////////

cGameCameraController::cGameCameraController()
 : m_pitch(kDefaultPitch)
 , m_oneOverTangentPitch(0)
 , m_elevation(kDefaultElevation)
 , m_focus(0,0,0)
 , m_velocity(0,0,0)
{
   ConfigGet(_T("view_elevation"), &m_elevation);
   ConfigGet(_T("view_pitch"), &m_pitch);
   MatrixRotateX(m_pitch, &m_rotation);
   m_oneOverTangentPitch = 1.0f / tanf(m_pitch);
}

///////////////////////////////////////

cGameCameraController::~cGameCameraController()
{
}

///////////////////////////////////////

void cGameCameraController::Connect()
{
   UseGlobal(Sim);
   pSim->Connect(this);
   UseGlobal(Input);
   pInput->Connect(this);
}

///////////////////////////////////////

void cGameCameraController::Disconnect()
{
   UseGlobal(Input);
   pInput->Disconnect(this);
   UseGlobal(Sim);
   pSim->Disconnect(this);
}

///////////////////////////////////////
// Very simple third-person camera model. Always looking down the -z axis
// and slightly pitched over the x axis.

void cGameCameraController::OnSimFrame(double elapsedTime)
{
   m_focus += m_velocity * (float)elapsedTime;

   float zOffset = m_elevation * m_oneOverTangentPitch;

   m_eye = tVec3(m_focus.x, m_focus.y + m_elevation, m_focus.z + zOffset);

   tMatrix4 mt;
   MatrixTranslate(-m_eye.x, -m_eye.y, -m_eye.z, &mt);

   tMatrix4 newModelView;
   m_rotation.Multiply(mt, &newModelView);

   UseGlobal(Camera);
   pCamera->SetViewMatrix(newModelView);
}

///////////////////////////////////////

bool cGameCameraController::OnInputEvent(const sInputEvent * pEvent)
{
   bool bUpdateCamera = false;

   switch (pEvent->key)
   {
      case kLeft:
      {
         m_velocity.x = pEvent->down ? -kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kRight:
      {
         m_velocity.x = pEvent->down ? kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kUp:
      {
         m_velocity.z = pEvent->down ? -kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kDown:
      {
         m_velocity.z = pEvent->down ? kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kMouseWheelUp:
      {
         m_elevation++;
         bUpdateCamera = true;
         break;
      }

      case kMouseWheelDown:
      {
         m_elevation--;
         bUpdateCamera = true;
         break;
      }
   }

   return bUpdateCamera;
}

///////////////////////////////////////

void cGameCameraController::LookAtPoint(float x, float z)
{
   m_focus = tVec3(x, 0, z);
}

///////////////////////////////////////////////////////////////////////////////
