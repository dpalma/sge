///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scene.h"

#include "render.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScene
//

///////////////////////////////////////

cScene::cScene()
{
}

///////////////////////////////////////

cScene::~cScene()
{
   Clear();
}

///////////////////////////////////////

tResult cScene::AddEntity(eSceneLayer layer, ISceneEntity * pEntity)
{
   return m_layers[layer].AddEntity(pEntity);
}

///////////////////////////////////////

tResult cScene::RemoveEntity(eSceneLayer layer, ISceneEntity * pEntity)
{
   return m_layers[layer].RemoveEntity(pEntity);
}

///////////////////////////////////////

tResult cScene::SetCamera(eSceneLayer layer, ISceneCamera * pCamera)
{
   return m_layers[layer].SetCamera(pCamera);
}

///////////////////////////////////////

tResult cScene::GetCamera(eSceneLayer layer, ISceneCamera * * ppCamera)
{
   return m_layers[layer].GetCamera(ppCamera);
}

///////////////////////////////////////

void cScene::Clear(eSceneLayer layer)
{
   m_layers[layer].Clear();
}

///////////////////////////////////////

void cScene::Clear()
{
   for (int i = 0; i < _countof(m_layers); i++)
   {
      m_layers[i].Clear();
   }
}

///////////////////////////////////////

tResult cScene::Render(IRenderDevice * pRenderDevice)
{
   for (int i = 0; i < _countof(m_layers); i++)
   {
      cAutoIPtr<ISceneCamera> pCamera;
      if (m_layers[i].GetCamera(&pCamera) == S_OK)
      {
         pRenderDevice->SetProjectionMatrix(pCamera->GetProjectionMatrix());
         pRenderDevice->SetViewMatrix(pCamera->GetViewMatrix());
      }
      tResult result = m_layers[i].Render(pRenderDevice);
      if (result != S_OK)
         return result;
   }
   return S_OK;
}

///////////////////////////////////////

tResult cScene::Query(const cRay & ray, tSceneEntityList * pEntities)
{
   Assert(pEntities != NULL);
   for (int i = 0; i < _countof(m_layers); i++)
   {
      m_layers[i].Query(ray, pEntities);
   }
   return pEntities->empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

void SceneCreate()
{
   cAutoIPtr<IScene>(new cScene);
}

///////////////////////////////////////////////////////////////////////////////
