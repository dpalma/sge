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
   MatrixPerspective(fov, aspect, znear, zfar, &m_projection);
   UpdateCompositeMatrices();
}

///////////////////////////////////////

void cSceneCamera::SetOrtho(float left, float right, float bottom, float top, float znear, float zfar)
{
   MatrixOrtho(left, right, bottom, top, znear, zfar, &m_projection);
   UpdateCompositeMatrices();
}

///////////////////////////////////////

void cSceneCamera::UpdateCompositeMatrices()
{
   m_viewProjection = GetProjectionMatrix() * GetViewMatrix();
   MatrixInvert(GetViewProjectionMatrix(), &m_viewProjectionInverse);
   m_frustum.ExtractPlanes(GetViewProjectionMatrix());
}

///////////////////////////////////////////////////////////////////////////////
