///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "render.h"
#include "scenecamera.h"

#include "dbgalloc.h" // must be last header

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
 : m_projectionType(kPT_Orthographic)
{
   m_projection.Identity();
   m_view.Identity();
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

void cSceneCamera::UpdateCompositeMatrices()
{
   m_viewProjection = GetProjectionMatrix() * GetViewMatrix();
   MatrixInvert(GetViewProjectionMatrix().m, m_viewProjectionInverse.m);
   m_frustum.ExtractPlanes(GetViewProjectionMatrix());
}

///////////////////////////////////////////////////////////////////////////////
