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

cSceneCamera::cSceneCamera()
{
   m_projection.Identity();
   m_modelView.Identity();
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
   m_modelViewProjection = GetProjectionMatrix() * GetModelViewMatrix();
   MatrixInvert(GetModelViewProjectionMatrix(), &m_modelViewProjectionInverse);
   m_frustum.ExtractPlanes(GetModelViewProjectionMatrix());
}

///////////////////////////////////////////////////////////////////////////////
