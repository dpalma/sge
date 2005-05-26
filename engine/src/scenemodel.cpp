///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenemodel.h"

#include "resourceapi.h"
#include "globalobj.h"

#include <windows.h> // HACK
#include <GL/gl.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneModel
//

ISceneEntity * SceneEntityCreate(const tChar * pszModel)
{
   if (pszModel == NULL || *pszModel == 0)
   {
      return NULL;
   }
   return static_cast<ISceneEntity *>(new cSceneModel(pszModel));
}

///////////////////////////////////////

cSceneModel::cSceneModel(const tChar * pszModel)
 : m_simClient(this),
   m_pSceneEntity(SceneEntityCreate()),
   m_model(pszModel),
   m_pModel(NULL),
   m_animationTime(0),
   m_animationLength(0)
{
   UseGlobal(Sim);
   pSim->Connect(&m_simClient);
}

///////////////////////////////////////

cSceneModel::~cSceneModel()
{
   UseGlobal(Sim);
   pSim->Disconnect(&m_simClient);

   delete m_pModel, m_pModel = NULL;
}

///////////////////////////////////////

void cSceneModel::Animate(double elapsedTime)
{
   if (m_pModel != NULL && m_pModel->IsAnimated())
   {
      m_animationTime += elapsedTime;
      while (m_animationTime > m_animationLength)
      {
         m_animationTime -= m_animationLength;
      }

      m_pModel->InterpolateJointMatrices(m_animationTime, &m_blendMatrices);
      m_pModel->ApplyJointMatrices(m_blendMatrices, &m_blendedVerts);
   }
}

///////////////////////////////////////

cSceneModel::cSimClient::cSimClient(cSceneModel * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

void cSceneModel::cSimClient::OnFrame(double elapsedTime)
{
   Assert(m_pOuter != NULL);
   m_pOuter->Animate(elapsedTime);
}

///////////////////////////////////////

void cSceneModel::Render(IRenderDevice * /*pRenderDevice*/)
{
   if (m_pModel == NULL)
   {
      UseGlobal(ResourceManager);
      if (pResourceManager->LoadUncached(m_model.c_str(), kRT_Model, NULL, (void**)&m_pModel, NULL) != S_OK)
      {
         return;
      }

      m_animationLength = m_pModel->GetTotalAnimationLength();
   }

   glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
   glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

   if (!m_blendedVerts.empty())
   {
      GlSubmitBlendedVertices(m_blendedVerts);
   }
   else
   {
      GlSubmitModelVertices(m_pModel->GetVertices());
   }

   tModelMeshes::const_iterator iter = m_pModel->BeginMeshses();
   tModelMeshes::const_iterator end = m_pModel->EndMeshses();
   for (; iter != end; iter++)
   {
      int iMaterial = iter->GetMaterialIndex();
      if (iMaterial >= 0)
      {
         m_pModel->GetMaterial(iMaterial).GlDiffuseAndTexture();
      }

#ifdef _DEBUG
      if (GlValidateIndices(iter->GetIndexData(), iter->GetIndexCount(),
         !m_blendedVerts.empty() ? m_blendedVerts.size() : m_pModel->GetVertices().size()))
      {
         glDrawElements(iter->GetGlPrimitive(), iter->GetIndexCount(), GL_UNSIGNED_SHORT, iter->GetIndexData());
      }
#else
      glDrawElements(iter->GetGlPrimitive(), iter->GetIndexCount(), GL_UNSIGNED_SHORT, iter->GetIndexData());
#endif
   }

   glPopClientAttrib();
   glPopAttrib();

}

///////////////////////////////////////////////////////////////////////////////
