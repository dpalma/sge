///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderapi.h"
#include "scenecamera.h"
#include "ray.h"

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://www.opengl.org/resources/faq/technical/viewing.htm

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneCamera
//

///////////////////////////////////////

ISceneCamera * SceneCameraCreate()
{
   return static_cast<ISceneCamera *>(new cSceneCamera);
}

///////////////////////////////////////

cSceneCamera::cSceneCamera()
 : m_projectionType(kPT_Orthographic),
   m_projection(tMatrix4::GetIdentity()),
   m_view(tMatrix4::GetIdentity())
{
}

///////////////////////////////////////

cSceneCamera::~cSceneCamera()
{
}

///////////////////////////////////////

void cSceneCamera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
   m_projectionType = kPT_Perspective;
   MatrixPerspective(fov, aspect, znear, zfar, &m_projection);
   UpdateCompositeMatrices();
}

///////////////////////////////////////

void cSceneCamera::SetOrtho(float left, float right, float bottom, float top, float znear, float zfar)
{
   m_projectionType = kPT_Orthographic;
   MatrixOrtho(left, right, bottom, top, znear, zfar, &m_projection);
   UpdateCompositeMatrices();
}

///////////////////////////////////////

tResult cSceneCamera::GeneratePickRay(float ndx, float ndy, cRay * pRay) const
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

   tMatrix4 inverseView;
   MatrixInvert(GetViewMatrix().m, inverseView.m);
   tVec4 eye;
   inverseView.Transform(tVec4(0,0,0,1), &eye);

   tVec3 dir(f.x - n.x, f.y - n.y, f.z - n.z);
   dir.Normalize();

   *pRay = cRay(tVec3(eye.x,eye.y,eye.z), dir);

   return S_OK;
}

///////////////////////////////////////

void cSceneCamera::UpdateCompositeMatrices()
{
   GetProjectionMatrix().Multiply(GetViewMatrix(), &m_viewProjection);
   MatrixInvert(GetViewProjectionMatrix().m, m_viewProjectionInverse.m);
   m_frustum.ExtractPlanes(GetViewProjectionMatrix());
}

///////////////////////////////////////////////////////////////////////////////
