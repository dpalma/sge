///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderglcamera.h"

#include "platform/sys.h"

#include "tech/configapi.h"
#include "tech/frustum.h"
#include "tech/matrix4.inl"
#include "tech/ray.inl"

#include <GL/glew.h>

#include "tech/dbgalloc.h" // must be last header

static const float kDefaultFov = 70;
static const float kDefaultZNear = 1;
static const float kDefaultZFar = 2000;


// REFERENCES
// http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf


///////////////////////////////////////////////////////////////////////////////

void ExtractFrustumPlanes(const tMatrix4 & viewProjection, cFrustum * pFrustum)
{
   tPlane planes[kMaxFrustumPlanes];

   // Determine the left plane
   planes[kFP_Left].a = viewProjection.m[3] + viewProjection.m[0];
   planes[kFP_Left].b = viewProjection.m[7] + viewProjection.m[4];
   planes[kFP_Left].c = viewProjection.m[11] + viewProjection.m[8];
   planes[kFP_Left].d = viewProjection.m[15] + viewProjection.m[12];
   planes[kFP_Left].Normalize();

   // Determine the right plane
   planes[kFP_Right].a = viewProjection.m[3] - viewProjection.m[0];
   planes[kFP_Right].b = viewProjection.m[7] - viewProjection.m[4];
   planes[kFP_Right].c = viewProjection.m[11] - viewProjection.m[8];
   planes[kFP_Right].d = viewProjection.m[15] - viewProjection.m[12];
   planes[kFP_Right].Normalize();

   // Determine the bottom plane
   planes[kFP_Bottom].a = viewProjection.m[3] + viewProjection.m[1];
   planes[kFP_Bottom].b = viewProjection.m[7] + viewProjection.m[5];
   planes[kFP_Bottom].c = viewProjection.m[11] + viewProjection.m[9];
   planes[kFP_Bottom].d = viewProjection.m[15] + viewProjection.m[13];
   planes[kFP_Bottom].Normalize();

   // Determine the top plane
   planes[kFP_Top].a = viewProjection.m[3] - viewProjection.m[1];
   planes[kFP_Top].b = viewProjection.m[7] - viewProjection.m[5];
   planes[kFP_Top].c = viewProjection.m[11] - viewProjection.m[9];
   planes[kFP_Top].d = viewProjection.m[15] - viewProjection.m[13];
   planes[kFP_Top].Normalize();

   // Determine the near plane
   planes[kFP_Near].a = viewProjection.m[3] + viewProjection.m[2];
   planes[kFP_Near].b = viewProjection.m[7] + viewProjection.m[6];
   planes[kFP_Near].c = viewProjection.m[11] + viewProjection.m[10];
   planes[kFP_Near].d = viewProjection.m[15] + viewProjection.m[14];
   planes[kFP_Near].Normalize();

   // Determine the far plane
   planes[kFP_Far].a = viewProjection.m[3] - viewProjection.m[2];
   planes[kFP_Far].b = viewProjection.m[7] - viewProjection.m[6];
   planes[kFP_Far].c = viewProjection.m[11] - viewProjection.m[10];
   planes[kFP_Far].d = viewProjection.m[15] - viewProjection.m[14];
   planes[kFP_Far].Normalize();

   *pFrustum = cFrustum(planes);
}


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
 : m_fov(kDefaultFov)
 , m_aspect(0)
 , m_znear(kDefaultZNear)
 , m_zfar(kDefaultZFar)
 , m_flags(kAutoAspect | kUpdateCompositeMatrices | kUpdateProjectionMatrix)
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

   m_flags |= kUpdateCompositeMatrices;

   memcpy(m_view, viewMatrix, 16 * sizeof(float));
   return S_OK;
}

////////////////////////////////////////

const float * cRenderGLCamera::GetProjectionMatrix() const
{
   return m_proj.m;
}

////////////////////////////////////////

tResult cRenderGLCamera::GetProjectionMatrix(float projMatrix[16]) const
{
   if (projMatrix == NULL)
   {
      return E_POINTER;
   }

   memcpy(projMatrix, m_proj.m, 16 * sizeof(float));
   return S_OK;
}

////////////////////////////////////////

tResult cRenderGLCamera::SetProjectionMatrix(const float projMatrix[16])
{
   if (projMatrix == NULL)
   {
      return E_POINTER;
   }

   m_flags |= kUpdateCompositeMatrices;
   m_flags &= ~kUpdateProjectionMatrix;

   memcpy(m_proj.m, projMatrix, 16 * sizeof(float));
   return S_OK;
}

////////////////////////////////////////

float cRenderGLCamera::GetFOV() const
{
   return m_fov;
}

////////////////////////////////////////

void cRenderGLCamera::SetFOV(float fov)
{
   m_fov = fov;
   m_flags |= kUpdateProjectionMatrix;
}

////////////////////////////////////////

float cRenderGLCamera::GetAspect() const
{
   if ((m_flags & kAutoAspect) == kAutoAspect)
   {
      int w = 0, h = 0;
      if (SysGetWindowSize(&w, &h) != S_OK)
      {
         ErrorMsg("SysGetWindowSize failed\n");
         return 1;
      }
      return static_cast<float>(w) / h;
   }
   else
   {
      return m_aspect;
   }
}

////////////////////////////////////////

void cRenderGLCamera::SetAspect(float aspect)
{
   m_aspect = aspect;
   m_flags &= ~kAutoAspect;
   m_flags |= kUpdateProjectionMatrix;
}

////////////////////////////////////////

void cRenderGLCamera::SetAutoAspect()
{
   m_flags |= (kAutoAspect | kUpdateProjectionMatrix);
}

////////////////////////////////////////

tResult cRenderGLCamera::GetNearFar(float * pZNear, float * pZFar) const
{
   if (pZNear == NULL || pZFar == NULL)
   {
      return E_NOTIMPL;
   }
   *pZNear = m_znear;
   *pZFar = m_zfar;
   return S_OK;
}

////////////////////////////////////////

tResult cRenderGLCamera::SetNearFar(float zNear, float zFar)
{
   m_znear = zNear;
   m_zfar = zFar;
   m_flags |= kUpdateProjectionMatrix;
   return S_OK;
}

////////////////////////////////////////

void cRenderGLCamera::UpdateCompositeMatrices() const
{
   if ((m_flags & kUpdateCompositeMatrices) == kUpdateCompositeMatrices)
   {
      float projMatrix[16], viewMatrix[16];
      GetProjectionMatrix(projMatrix);
      GetViewMatrix(viewMatrix);
      MatrixMultiply(projMatrix, viewMatrix, m_viewProj);
      MatrixInvert(m_viewProj, m_viewProjInv);
      m_flags &= ~kUpdateCompositeMatrices;
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

tResult cRenderGLCamera::GeneratePickRay(float ndx, float ndy, cRay<float> * pRay) const
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

   tVec4 n = m.Transform(tVec4(ndx, ndy, -1, 1));
   if (n.w == 0.0f)
   {
      return E_FAIL;
   }
   n.x /= n.w;
   n.y /= n.w;
   n.z /= n.w;

   tVec4 f = m.Transform(tVec4(ndx, ndy, 1, 1));
   if (f.w == 0.0f)
   {
      return E_FAIL;
   }
   f.x /= f.w;
   f.y /= f.w;
   f.z /= f.w;

   cMatrix4<float> viewInv(m_viewInv);
   cPoint3<float> eye = viewInv.Transform(cPoint3<float>(0,0,0));

   tVec3 dir(f.x - n.x, f.y - n.y, f.z - n.z);
   dir.Normalize();

   *pRay = cRay<float>(cPoint3<float>(eye.x,eye.y,eye.z), dir);

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
