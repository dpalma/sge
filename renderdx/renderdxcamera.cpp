///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderdxcamera.h"

#include "tech/matrix4.h"
#include "tech/ray.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderDXCamera
//

////////////////////////////////////////

tResult RenderCameraCreate(IRenderCamera * * ppRenderCamera)
{
   if (ppRenderCamera == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IRenderCamera> pCamera(static_cast<IRenderCamera*>(new cRenderDXCamera));
   if (!pCamera)
   {
      return E_OUTOFMEMORY;
   }

   return pCamera.GetPointer(ppRenderCamera);
}

////////////////////////////////////////

cRenderDXCamera::cRenderDXCamera()
{
}

////////////////////////////////////////

cRenderDXCamera::~cRenderDXCamera()
{
}

////////////////////////////////////////

const float * cRenderDXCamera::GetViewMatrix() const
{
   return NULL;
}

////////////////////////////////////////

tResult cRenderDXCamera::GetViewMatrix(float viewMatrix[16]) const
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRenderDXCamera::SetViewMatrix(const float viewMatrix[16])
{
   return E_NOTIMPL;
}

////////////////////////////////////////

const float * cRenderDXCamera::GetProjectionMatrix() const
{
   return NULL;
}

////////////////////////////////////////

tResult cRenderDXCamera::GetProjectionMatrix(float projMatrix[16]) const
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRenderDXCamera::SetProjectionMatrix(const float projMatrix[16])
{
   return E_NOTIMPL;
}

////////////////////////////////////////

float cRenderDXCamera::GetFOV() const
{
   return 70;
}

////////////////////////////////////////

void cRenderDXCamera::SetFOV(float fov)
{
}

////////////////////////////////////////

float cRenderDXCamera::GetAspect() const
{
   return 1;
}

////////////////////////////////////////

void cRenderDXCamera::SetAspect(float aspect)
{
}

////////////////////////////////////////

void cRenderDXCamera::SetAutoAspect()
{
}

////////////////////////////////////////

tResult cRenderDXCamera::GetNearFar(float * pZNear, float * pZFar) const
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRenderDXCamera::SetNearFar(float zNear, float zFar)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRenderDXCamera::GetViewProjectionMatrix(float viewProjMatrix[16]) const
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRenderDXCamera::GetViewProjectionInverseMatrix(float viewProjInvMatrix[16]) const
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRenderDXCamera::ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy) const
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cRenderDXCamera::GeneratePickRay(float ndx, float ndy, cRay<float> * pRay) const
{
   return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
