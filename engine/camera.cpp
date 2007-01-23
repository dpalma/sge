///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "camera.h"

#include "platform/keys.h"
#include "render/renderapi.h"
#include "tech/readwriteutils.h"
#include "tech/configapi.h"

#include "tech/dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' : used in base member initializer list

// REFERENCES
// http://www.opengl.org/resources/faq/technical/viewing.htm


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCameraControl
//

static const float kElevationDefault = 100;
static const float kElevationMin = 25;
static const float kElevationMax = 200;
static const float kElevationStep = 10;
static const float kElevationSpeed = 15;
static const float kDefaultPitch = 70;
static const float kDefaultSpeed = 50;

const int cCameraControl::gm_currentSaveLoadVer = 1;

///////////////////////////////////////

cCameraControl::cCameraControl()
 : m_moveCameraTask(this)
 , m_cameraMove(kCameraMoveNone)
 , m_pitch(kDefaultPitch)
 , m_oneOverTangentPitch(0)
 , m_elevation(kElevationDefault)
 , m_focus(0,0,0)
 , m_elevationLerp(kElevationDefault, kElevationDefault, 1)
 , m_bPitchChanged(false)
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
   UseGlobal(Scheduler);
   pScheduler->AddFrameTask(&m_moveCameraTask, 0, 1, 0);

   UseGlobal(Input);
   pInput->AddInputListener(static_cast<IInputListener*>(this));

   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RegisterSaveLoadParticipant(SAVELOADID_CameraControl,
      gm_currentSaveLoadVer, static_cast<ISaveLoadParticipant*>(this));

   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::Term()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RevokeSaveLoadParticipant(SAVELOADID_CameraControl, gm_currentSaveLoadVer);

   UseGlobal(Input);
   pInput->RemoveInputListener(static_cast<IInputListener*>(this));

   UseGlobal(Scheduler);
   pScheduler->RemoveFrameTask(&m_moveCameraTask);

   return S_OK;
}

///////////////////////////////////////

bool cCameraControl::OnInputEvent(const sInputEvent * pEvent)
{
   switch (pEvent->key)
   {
      case kLeft:
      {
         if (pEvent->down)
         {
            m_cameraMove |= kCameraMoveLeft;
         }
         else
         {
            m_cameraMove &= ~kCameraMoveLeft;
         }
         break;
      }

      case kRight:
      {
         if (pEvent->down)
         {
            m_cameraMove |= kCameraMoveRight;
         }
         else
         {
            m_cameraMove &= ~kCameraMoveRight;
         }
         break;
      }

      case kUp:
      {
         if (pEvent->down)
         {
            m_cameraMove |= kCameraMoveForward;
         }
         else
         {
            m_cameraMove &= ~kCameraMoveForward;
         }
         break;
      }

      case kDown:
      {
         if (pEvent->down)
         {
            m_cameraMove |= kCameraMoveBack;
         }
         else
         {
            m_cameraMove &= ~kCameraMoveBack;
         }
         break;
      }

      case kMouseWheelUp:
      {
         if (pEvent->down)
         {
            Raise();
         }
         break;
      }

      case kMouseWheelDown:
      {
         if (pEvent->down)
         {
            Lower();
         }
         break;
      }
   }

   return false;
}

///////////////////////////////////////

static const uint kCameraMoveLeftRight = (kCameraMoveLeft | kCameraMoveRight);
static const uint kCameraMoveForwardBack = (kCameraMoveForward | kCameraMoveBack);

void cCameraControl::SimFrame(double elapsedTime)
{
   if (m_cameraMove != kCameraMoveNone)
   {
      float focusVelX = 0, focusVelZ = 0;

      if ((m_cameraMove & kCameraMoveLeftRight) == kCameraMoveLeft)
      {
         focusVelX = -kDefaultSpeed;
      }
      else if ((m_cameraMove & kCameraMoveLeftRight) == kCameraMoveRight)
      {
         focusVelX = kDefaultSpeed;
      }

      if ((m_cameraMove & kCameraMoveForwardBack) == kCameraMoveForward)
      {
         focusVelZ = -kDefaultSpeed;
      }
      else if ((m_cameraMove & kCameraMoveForwardBack) == kCameraMoveBack)
      {
         focusVelZ = kDefaultSpeed;
      }

      m_focus.x += static_cast<float>(focusVelX * elapsedTime);
      m_focus.z += static_cast<float>(focusVelZ * elapsedTime);
   }

   if (m_bPitchChanged)
   {
      MatrixRotateX(m_pitch, &m_rotation);
      m_oneOverTangentPitch = 1.0f / tanf(m_pitch);
      m_bPitchChanged = false;
   }

   m_elevation = m_elevationLerp.Update(static_cast<float>(elapsedTime));

   float zOffset = m_elevation * m_oneOverTangentPitch;

   m_eye = tVec3(m_focus.x, m_focus.y + m_elevation, m_focus.z + zOffset);

   tMatrix4 mt;
   MatrixTranslate(-m_eye.x, -m_eye.y, -m_eye.z, &mt);

   tMatrix4 newModelView;
   m_rotation.Multiply(mt, &newModelView);

   UseGlobal(Renderer);
   cAutoIPtr<IRenderCamera> pCamera;
   if (pRenderer->GetCamera(&pCamera) == S_OK)
   {
      pCamera->SetViewMatrix(newModelView.m);
   }
}

///////////////////////////////////////

tResult cCameraControl::GetElevation(float * pElevation) const
{
   if (pElevation == NULL)
   {
      return E_POINTER;
   }
   *pElevation = m_elevation;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::SetElevation(float elevation)
{
   m_elevation = elevation;
   m_elevationLerp.Restart(m_elevation, m_elevation, 1);
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::GetPitch(float * pPitch) const
{
   if (pPitch == NULL)
   {
      return E_POINTER;
   }
   *pPitch = m_pitch;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::SetPitch(float pitch)
{
   m_pitch = pitch;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::LookAtPoint(float x, float z)
{
   m_focus = tVec3(x, 0, z);
   return S_OK;
}

///////////////////////////////////////

void cCameraControl::SetMovement(uint mask, uint flag)
{
   m_cameraMove = (m_cameraMove & ~mask) | (flag & mask);
}

///////////////////////////////////////

tResult cCameraControl::Raise()
{
   m_elevationLerp.Restart(m_elevation, m_elevation + kElevationStep, kElevationSpeed);
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::Lower()
{
   m_elevationLerp.Restart(m_elevation, m_elevation - kElevationStep, kElevationSpeed);
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::Save(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(m_pitch) != S_OK
      || pWriter->Write(m_elevation) != S_OK
      || pWriter->Write(m_eye) != S_OK
      || pWriter->Write(m_focus) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::Load(IReader * pReader, int version)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   if (version != gm_currentSaveLoadVer)
   {
      // Would eventually handle upgrading here
      return E_FAIL;
   }

   if (pReader->Read(&m_pitch) != S_OK
      || pReader->Read(&m_elevation) != S_OK
      || pReader->Read(&m_eye) != S_OK
      || pReader->Read(&m_focus) != S_OK)
   {
      return E_FAIL;
   }

   m_rotation = tMatrix4::GetIdentity();
   m_oneOverTangentPitch = 0;
   m_elevationLerp.Restart(m_elevation, m_elevation, 0);
   m_cameraMove = kCameraMoveNone;
   m_bPitchChanged = true;

   return S_OK;
}

///////////////////////////////////////

void cCameraControl::Reset()
{
   m_cameraMove = kCameraMoveNone;
   m_pitch = kDefaultPitch;
   m_oneOverTangentPitch = 0;
   m_elevation = kElevationDefault;
   m_focus = tVec3(0,0,0);
   m_elevationLerp.Restart(kElevationDefault, kElevationDefault, 1);
   m_bPitchChanged = false;
   ConfigGet(_T("view_elevation"), &m_elevation);
   ConfigGet(_T("view_pitch"), &m_pitch);
   MatrixRotateX(m_pitch, &m_rotation);
   m_oneOverTangentPitch = 1.0f / tanf(m_pitch);
}

///////////////////////////////////////

cCameraControl::cMoveCameraTask::cMoveCameraTask(cCameraControl * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

tResult cCameraControl::cMoveCameraTask::Execute(double time)
{
   double elapsed = fabs(time - m_lastTime);
   m_pOuter->SimFrame(elapsed);
   m_lastTime = time;
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
