///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scenemodel.h"
#include "model.h"

#include "matrix4.h"
#include "resourceapi.h"
#include "globalobj.h"

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
   m_animationTime(0)
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
   }

   m_pModel->Render();
}

///////////////////////////////////////////////////////////////////////////////
