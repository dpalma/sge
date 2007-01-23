///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderglcamera.h"

#include "tech/matrix4.h"
#include "tech/ray.h"

#include <GL/glew.h>

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderGLCamera
//

////////////////////////////////////////

tResult RenderCameraCreate(IRenderCamera * * ppRenderCamera)
{
   if (ppRenderCamera == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IRenderCamera> pCamera(static_cast<IRenderCamera*>(new cRenderGLCamera));
   if (!pCamera)
   {
      return E_OUTOFMEMORY;
   }

   return pCamera.GetPointer(ppRenderCamera);
}

////////////////////////////////////////

cRenderGLCamera::cRenderGLCamera()
 : m_bUpdateCompositeMatrices(true)
{
}

////////////////////////////////////////

cRenderGLCamera::~cRenderGLCamera()
{
}

////////////////////////////////////////

const float * cRenderGLCamera::GetViewMatrix() const
{
   return m_view;
}

////////////////////////////////////////

tResult cRenderGLCamera::GetViewMatrix(float viewMatrix[16]) const
{
   if (viewMatrix == NULL)
   {
      return E_POINTER;
   }

   memcpy(viewMatrix, m_view, 16 * sizeof(float));
   return S_OK;
}

////////////////////////////////////////

tResult cRenderGLCamera::SetViewMatrix(const float viewMatrix[16])
{
   if (viewMatrix == NULL)
   {
      return E_POINTER;
   }

   MatrixInvert(viewMatrix, m_viewInv);

   m_bUpdateCompositeMatrices = true;

   memcpy(m_view, viewMatrix, 16 * sizeof(float));
   return S_OK;
}

////////////////////////////////////////

const float * cRenderGLCamera::GetProjectionMatrix() const
{
   return m_proj;
}

////////////////////////////////////////

tResult cRenderGLCamera::GetProjectionMatrix(float projMatrix[16]) const
{
   if (projMatrix == NULL)
   {
      return E_POINTER;
   }

   memcpy(projMatrix, m_proj, 16 * sizeof(float));
   return S_OK;
}

////////////////////////////////////////

tResult cRenderGLCamera::SetProjectionMatrix(const float projMatrix[16])
{
   if (projMatrix == NULL)
   {
      return E_POINTER;
   }

   m_bUpdateCompositeMatrices = true;

   memcpy(m_proj, projMatrix, 16 * sizeof(float));
   return S_OK;
}

////////////////////////////////////////

void cRenderGLCamera::UpdateCompositeMatrices() const
{
   if (m_bUpdateCompositeMatrices)
   {
      float projMatrix[16], viewMatrix[16];
      GetProjectionMatrix(projMatrix);
      GetViewMatrix(viewMatrix);
      MatrixMultiply(projMatrix, viewMatrix, m_viewProj);
      MatrixInvert(m_viewProj, m_viewProjInv);
      m_bUpdateCompositeMatrices = false;
   }
}

////////////////////////////////////////

tResult cRenderGLCamera::GetViewProjectionMatrix(float viewProjMatrix[16]) const
{
   if (viewProjMatrix == NULL)
   {
      return E_POINTER;
   }

   UpdateCompositeMatrices();

   memcpy(viewProjMatrix, m_viewProj, 16 * sizeof(float));
   return S_OK;
}

////////////////////////////////////////

tResult cRenderGLCamera::GetViewProjectionInverseMatrix(float viewProjInvMatrix[16]) const
{
   if (viewProjInvMatrix == NULL)
   {
      return E_POINTER;
   }

   UpdateCompositeMatrices();

   memcpy(viewProjInvMatrix, m_viewProjInv, 16 * sizeof(float));
   return S_OK;
}

////////////////////////////////////////

tResult cRenderGLCamera::ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy) const
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

///////////////////////////////////////

tResult cRenderGLCamera::GeneratePickRay(float ndx, float ndy, cRay * pRay) const
{
   if (pRay == NULL)
   {
      return E_POINTER;
   }

   tMatrix4 m;
   if (GetViewProjectionInverseMatrix(m.m) != S_OK)
   {
      return E_FAIL;
   }

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
   MatrixTransform4(m_viewInv, tVec4(0,0,0,1).v, eye.v);

   tVec3 dir(f.x - n.x, f.y - n.y, f.z - n.z);
   dir.Normalize();

   *pRay = cRay(tVec3(eye.x,eye.y,eye.z), dir);

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
