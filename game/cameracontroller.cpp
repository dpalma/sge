///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "cameracontroller.h"
#include "globalobj.h"
#include "ray.h"
#include "ggl.h"
#include "sceneapi.h"

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

/////////////////////////////////////////////////////////////////////////////

static void ScreenToNormalizedDeviceCoords(int sx, int sy,
                                           float * pndx, float * pndy)
{
   Assert(pndx != NULL);
   Assert(pndy != NULL);

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   sy = viewport[3] - sy;

   // convert screen coords to normalized (origin at center, [-1..1])
   float normx = (float)(sx - viewport[0]) * 2.f / viewport[2] - 1.f;
   float normy = (float)(sy - viewport[1]) * 2.f / viewport[3] - 1.f;

   *pndx = normx;
   *pndy = normy;
}

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
   m_pCamera(CTAddRef(pCamera))
{
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

   tMatrix4 newModelView;
   m_rotation.Multiply(mt, &newModelView);
   m_pCamera->SetViewMatrix(newModelView);
}

///////////////////////////////////////

bool cGameCameraController::OnInputEvent(const sInputEvent * pEvent)
{
   if ((pEvent->key == kMouseLeft) && pEvent->down)
   {
      float ndx, ndy;
      ScreenToNormalizedDeviceCoords(Round(pEvent->point.x), Round(pEvent->point.y), &ndx, &ndy);

      Assert(m_pCamera != NULL);

      cRay ray;
      if (m_pCamera->GeneratePickRay(ndx, ndy, &ray) == S_OK)
      {
         cAutoIPtr<ISceneEntityEnum> pHits;

         UseGlobal(Scene);
         if (pScene->Query(ray, &pHits) == S_OK)
         {
            return true;
         }
#ifdef _DEBUG
         else
         {
            tVec3 intersect;
            if (ray.IntersectsPlane(tVec3(0,1,0), 0, &intersect))
            {
               DebugMsg3("Hit the ground at approximately (%.1f,%.1f,%.1f)\n",
                  intersect.x, intersect.y, intersect.z);
            }
         }
#endif
      }
   }

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
