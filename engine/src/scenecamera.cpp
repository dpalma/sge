///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "render.h"
#include "scenecamera.h"
#include "gcommon.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneCameraGroup
//

///////////////////////////////////////

cSceneCameraGroup::cSceneCameraGroup()
{
   m_projection.Identity();
   m_modelView.Identity();
}

///////////////////////////////////////

cSceneCameraGroup::~cSceneCameraGroup()
{
}

///////////////////////////////////////

void cSceneCameraGroup::SetPerspective(float fov, float aspect, float znear, float zfar)
{
   MatrixPerspective(fov, aspect, znear, zfar, &m_projection);
   UpdateCompositeMatrices();
}

///////////////////////////////////////

void cSceneCameraGroup::SetOrtho(float left, float right, float bottom, float top, float znear, float zfar)
{
   MatrixOrtho(left, right, bottom, top, znear, zfar, &m_projection);
   UpdateCompositeMatrices();
}

///////////////////////////////////////

void cSceneCameraGroup::UpdateCompositeMatrices()
{
   m_modelViewProjection = GetProjectionMatrix() * GetModelViewMatrix();
   MatrixInvert(GetModelViewProjectionMatrix(), &m_modelViewProjectionInverse);
   m_frustum.ExtractPlanes(GetModelViewProjectionMatrix());
}

///////////////////////////////////////

void cSceneCameraGroup::Render()
{
   AccessRenderDevice()->SetProjectionMatrix(GetProjectionMatrix());
   AccessRenderDevice()->SetViewMatrix(GetModelViewMatrix());
   cSceneGroup::Render();
}

///////////////////////////////////////////////////////////////////////////////
