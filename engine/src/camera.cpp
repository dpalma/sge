///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "camera.h"

#include "configapi.h"
#include "keys.h"
#include "ray.h"
#include "vec4.h"

#include <algorithm>

#include <GL/glew.h>

#include "dbgalloc.h" // must be last header


// REFERENCES
// http://www.opengl.org/resources/faq/technical/viewing.htm

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCamera
//

///////////////////////////////////////

cCamera::cCamera()
 : m_bUpdateCompositeMatrices(true)
{
}

///////////////////////////////////////

cCamera::~cCamera()
{
}

///////////////////////////////////////

tResult cCamera::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cCamera::Term()
{
   return S_OK;
}

///////////////////////////////////////

void cCamera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
   MatrixPerspective(fov, aspect, znear, zfar, &m_proj);
   m_bUpdateCompositeMatrices = true;
}

///////////////////////////////////////

void cCamera::SetOrtho(float left, float right, float bottom, float top, float znear, float zfar)
{
   MatrixOrtho(left, right, bottom, top, znear, zfar, &m_proj);
   m_bUpdateCompositeMatrices = true;
}

///////////////////////////////////////

tResult cCamera::GeneratePickRay(float ndx, float ndy, cRay * pRay) const
{
   if (pRay == NULL)
   {
      return E_POINTER;
   }

   const tMatrix4 & m = GetViewProjectionInverseMatrix();

   tVec4 n;
   m.Transform(tVec4(ndx, ndy, -1, 1), &n);
   if (n.w == 0.0f)
   {
      return E_FAIL;
   }
   n.x /= n.w;
   n.y /= n.w;
   n.z /= n.w;

   tVec4 f;
   m.Transform(tVec4(ndx, ndy, 1, 1), &f);
   if (f.w == 0.0f)
   {
      return E_FAIL;
   }
   f.x /= f.w;
   f.y /= f.w;
   f.z /= f.w;

   tVec4 eye;
   m_viewInv.Transform(tVec4(0,0,0,1), &eye);

   tVec3 dir(f.x - n.x, f.y - n.y, f.z - n.z);
   dir.Normalize();

   *pRay = cRay(tVec3(eye.x,eye.y,eye.z), dir);

   return S_OK;
}

///////////////////////////////////////

void cCamera::SetGLState()
{
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(GetProjectionMatrix().m);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf(GetViewMatrix().m);
}

///////////////////////////////////////

void cCamera::UpdateCompositeMatrices() const
{
   if (m_bUpdateCompositeMatrices)
   {
      m_bUpdateCompositeMatrices = false;
      MatrixInvert(m_view.m, m_viewInv.m);
      GetProjectionMatrix().Multiply(GetViewMatrix(), &m_viewProj);
      MatrixInvert(GetViewProjectionMatrix().m, m_viewProjInv.m);
      m_frustum.ExtractPlanes(GetViewProjectionMatrix());
   }
}

///////////////////////////////////////

tResult CameraCreate()
{
   cAutoIPtr<ICamera> p(static_cast<ICamera*>(new cCamera));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ICamera, p);
}

///////////////////////////////////////////////////////////////////////////////

tResult ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy)
{
   if (pndx == NULL || pndy == NULL)
   {
      return E_POINTER;
   }

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   sy = viewport[3] - sy;

   // convert screen coords to normalized (origin at center, [-1..1])
   float normx = (float)(sx - viewport[0]) * 2.f / viewport[2] - 1.f;
   float normy = (float)(sy - viewport[1]) * 2.f / viewport[3] - 1.f;

   *pndx = normx;
   *pndy = normy;

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCameraControl
//

static const float kDefaultElevation = 100;
static const float kDefaultPitch = 70;
static const float kDefaultSpeed = 50;

///////////////////////////////////////

cCameraControl::cCameraControl()
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

cCameraControl::~cCameraControl()
{
}

///////////////////////////////////////

tResult cCameraControl::Init()
{
   UseGlobal(Input);
   pInput->AddInputListener(static_cast<IInputListener*>(this));
   UseGlobal(Sim);
   pSim->Connect(static_cast<ISimClient*>(this));
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::Term()
{
   UseGlobal(Input);
   pInput->RemoveInputListener(static_cast<IInputListener*>(this));
   UseGlobal(Sim);
   pSim->Disconnect(static_cast<ISimClient*>(this));
   return S_OK;
}

///////////////////////////////////////

bool cCameraControl::OnInputEvent(const sInputEvent * pEvent)
{
   switch (pEvent->key)
   {
      case kLeft:
      {
         m_velocity.x = pEvent->down ? -kDefaultSpeed : 0;
         break;
      }

      case kRight:
      {
         m_velocity.x = pEvent->down ? kDefaultSpeed : 0;
         break;
      }

      case kUp:
      {
         m_velocity.z = pEvent->down ? -kDefaultSpeed : 0;
         break;
      }

      case kDown:
      {
         m_velocity.z = pEvent->down ? kDefaultSpeed : 0;
         break;
      }

      case kMouseWheelUp:
      {
         m_elevation++;
         break;
      }

      case kMouseWheelDown:
      {
         m_elevation--;
         break;
      }
   }

   return false;
}

///////////////////////////////////////

void cCameraControl::OnSimFrame(double elapsedTime)
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

tResult cCameraControl::LookAtPoint(float x, float z)
{
   m_focus = tVec3(x, 0, z);
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::MoveLeft()
{
   m_velocity.x = -kDefaultSpeed;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::MoveRight()
{
   m_velocity.x = kDefaultSpeed;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::MoveForward()
{
   m_velocity.z = -kDefaultSpeed;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::MoveBack()
{
   m_velocity.z = kDefaultSpeed;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::Raise()
{
   m_velocity.y = kDefaultSpeed;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::Lower()
{
   m_velocity.y = -kDefaultSpeed;
   return S_OK;
}

///////////////////////////////////////

tResult CameraControlCreate()
{
   cAutoIPtr<ICameraControl> pCameraControl(static_cast<ICameraControl*>(new cCameraControl));
   if (!pCameraControl)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ICameraControl, pCameraControl);
}

///////////////////////////////////////////////////////////////////////////////
