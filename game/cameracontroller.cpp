///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "cameracontroller.h"
#include "globalobj.h"
#include "ray.h"
#include "ggl.h"
#include "sceneapi.h"

#include "vec4.h"
#include "configapi.h"
#include "keys.h"
#include "globalobj.h"

#include <algorithm>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

static const float kDefaultElevation = 100;
static const float kDefaultPitch = 70;
static const float kDefaultSpeed = 50;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGameCameraController
//

///////////////////////////////////////

cGameCameraController::cGameCameraController(ISceneCamera * pCamera)
 : m_pitch(kDefaultPitch),
   m_oneOverTangentPitch(0),
   m_elevation(kDefaultElevation),
   m_focus(0,0,0),
   m_velocity(0,0,0),
   m_pCamera(pCamera)
{
   if (pCamera != NULL)
      pCamera->AddRef();
   ConfigGet("view_elevation", &m_elevation);
   ConfigGet("view_pitch", &m_pitch);
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
   UseGlobal(Scene);
   pScene->AddInputListener(kSL_Object, this);
}

///////////////////////////////////////

void cGameCameraController::Disconnect()
{
   UseGlobal(Scene);
   pScene->RemoveInputListener(kSL_Object, this);
   UseGlobal(Sim);
   pSim->Disconnect(this);
}

///////////////////////////////////////
// Very simple third-person camera model. Always looking down the -z axis
// and slightly pitched over the x axis.

void cGameCameraController::OnFrame(double elapsedTime)
{
   m_focus += m_velocity * (float)elapsedTime;

   float zOffset = m_elevation * m_oneOverTangentPitch;

   m_eye = tVec3(m_focus.x, m_focus.y + m_elevation, m_focus.z + zOffset);

   tMatrix4 mt;
   MatrixTranslate(-m_eye.x, -m_eye.y, -m_eye.z, &mt);

   tMatrix4 newModelView = m_rotation * mt;
   m_pCamera->SetViewMatrix(newModelView);
}

///////////////////////////////////////

bool cGameCameraController::OnMouseEvent(int x, int y, uint mouseState, double time)
{
   if (mouseState & kLMouseDown)
   {
      tVec3 dir;
      if (BuildPickRay(x, y, &dir))
      {
         cRay ray(GetEyePosition(), dir);

         cAutoIPtr<ISceneEntityEnum> pHits;

         UseGlobal(Scene);
         if (pScene->Query(ray, &pHits) == S_FALSE)
         {
            tVec3 intersect;
            if (ray.IntersectsPlane(tVec3(0,1,0), 0, &intersect))
            {
               DebugMsg3("Hit the ground at approximately (%.1f,%.1f,%.1f)\n",
                  intersect.x, intersect.y, intersect.z);
            }
         }

         return true;
      }
   }

   return false;
}

///////////////////////////////////////

bool cGameCameraController::OnKeyEvent(long key, bool down, double time)
{
   bool bUpdateCamera = false;

   switch (key)
   {
      case kLeft:
      {
         m_velocity.x = down ? -kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kRight:
      {
         m_velocity.x = down ? kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kUp:
      {
         m_velocity.z = down ? -kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kDown:
      {
         m_velocity.z = down ? kDefaultSpeed : 0;
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

///////////////////////////////////////

bool cGameCameraController::BuildPickRay(int x, int y, tVec3 * pRay)
{
   Assert(m_pCamera != NULL);

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   y = viewport[3] - y;

   // convert screen coords to normalized (origin at center, [-1..1])
   float normx = (float)(x - viewport[0]) * 2.f / viewport[2] - 1.f;
   float normy = (float)(y - viewport[1]) * 2.f / viewport[3] - 1.f;

   const tMatrix4 & m = m_pCamera->GetViewProjectionInverseMatrix();

   tVec4 n = m.Transform(tVec4(normx, normy, -1, 1));
   if (n.w == 0.0f)
      return false;
   n.x /= n.w;
   n.y /= n.w;
   n.z /= n.w;

   tVec4 f = m.Transform(tVec4(normx, normy, 1, 1));
   if (f.w == 0.0f)
      return false;
   f.x /= f.w;
   f.y /= f.w;
   f.z /= f.w;

   Assert(pRay != NULL);
   *pRay = tVec3(f.x - n.x, f.y - n.y, f.z - n.z);
   pRay->Normalize();

   return true;
}

///////////////////////////////////////////////////////////////////////////////
