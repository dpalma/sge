///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "camera.h"
#include "scenegraph.h"

#include "render.h"
#include "ggl.h"

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCameraRenderVisitor
//

class cCameraRenderVisitor : public cSceneNodeVisitor
{
public:
   virtual void VisitSceneNode(cSceneNode * pNode);
};

///////////////////////////////////////

void cCameraRenderVisitor::VisitSceneNode(cSceneNode * pNode)
{
   Assert(pNode != NULL);
   pNode->Render();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCamera
//

///////////////////////////////////////

cCamera::cCamera()
{
   m_projection.Identity();
   m_modelView.Identity();
}

///////////////////////////////////////

cCamera::~cCamera()
{
}

///////////////////////////////////////

void cCamera::Render(IRenderDevice * pDevice, cSceneNode * pScene)
{
   Assert(pDevice != NULL);
   Assert(pScene != NULL);

   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(GetProjectionMatrix().m);

   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf(GetModelViewMatrix().m);

   cCameraRenderVisitor renderVisitor;
   pScene->Traverse(&renderVisitor);
}

///////////////////////////////////////

void cCamera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
   MatrixPerspective(fov, aspect, znear, zfar, &m_projection);
   UpdateCompositeMatrices();
}

///////////////////////////////////////

void cCamera::SetOrtho(float left, float right, float bottom, float top, float znear, float zfar)
{
   MatrixOrtho(left, right, bottom, top, znear, zfar, &m_projection);
   UpdateCompositeMatrices();
}

///////////////////////////////////////

void cCamera::UpdateCompositeMatrices()
{
   m_modelViewProjection = GetProjectionMatrix() * GetModelViewMatrix();
   MatrixInvert(GetModelViewProjectionMatrix(), &m_modelViewProjectionInverse);
   m_frustum.ExtractPlanes(GetModelViewProjectionMatrix());
}

///////////////////////////////////////////////////////////////////////////////
