///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scene.h"
#include "frustum.h"

#include "renderapi.h"

#include "matrix4.h"

#include <algorithm>

// TODO: HACK
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "dbgalloc.h" // must be last header

F_DECLARE_INTERFACE(IInput);

extern ISceneEntityEnum * SceneEntityEnumCreate(const tSceneEntityList & entities);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScene
//

BEGIN_CONSTRAINTS()
   AFTER_GUID(IID_IInput)
END_CONSTRAINTS()

///////////////////////////////////////

cScene::cScene()
 : cGlobalObject<IMPLEMENTS(IScene)>("Scene", CONSTRAINTS()),
   m_inputListener(this)
{
}

///////////////////////////////////////

cScene::~cScene()
{
}

///////////////////////////////////////

tResult cScene::Init()
{
   UseGlobal(Input);
   pInput->Connect(&m_inputListener);
   return S_OK;
}

///////////////////////////////////////

tResult cScene::Term()
{
   UseGlobal(Input);
   pInput->Disconnect(&m_inputListener);

   Clear();

   return S_OK;
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

AssertOnce(kSL_Terrain == 0);
AssertOnce(kSL_Object == 1);
AssertOnce(kSL_InGameUI == 2);
AssertOnce(kSL_FogOfWar == 3);

tResult cScene::GetCamera(eSceneLayer layer, ISceneCamera * * ppCamera)
{
   for (int i = layer; i >= 0; i--)
   {
	   if (m_layers[i].GetCamera(ppCamera) == S_OK)
	   {
		   return S_OK;
	   }
   }
   return S_FALSE;
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

tResult cScene::AddInputListener(eSceneLayer layer, IInputListener * pListener)
{
   return m_layers[layer].AddInputListener(pListener);
}

///////////////////////////////////////

tResult cScene::RemoveInputListener(eSceneLayer layer, IInputListener * pListener)
{
   return m_layers[layer].RemoveInputListener(pListener);
}

///////////////////////////////////////

class cRenderEntity
{
public:
   cRenderEntity(IRenderDevice * pRenderDevice);

   void operator()(ISceneEntity * pEntity);

private:
   cAutoIPtr<IRenderDevice> m_pRenderDevice;
};

cRenderEntity::cRenderEntity(IRenderDevice * pRenderDevice)
 : m_pRenderDevice(CTAddRef(pRenderDevice))
{
   Assert(m_pRenderDevice != NULL);
}

void cRenderEntity::operator()(ISceneEntity * pEntity)
{
   glPushMatrix();
   glMultMatrixf(pEntity->GetWorldTransform().m);
   pEntity->Render(m_pRenderDevice);
   glPopMatrix();
}

tResult cScene::Render(IRenderDevice * pRenderDevice)
{
   cFrustum frustum;
   bool bHaveFrustum = false;

   for (int i = 0; i < _countof(m_layers); i++)
   {
      cAutoIPtr<ISceneCamera> pCamera;
      if (m_layers[i].GetCamera(&pCamera) == S_OK)
      {
         pRenderDevice->SetProjectionMatrix(pCamera->GetProjectionMatrix());
         pRenderDevice->SetViewMatrix(pCamera->GetViewMatrix());
         if (pCamera->GetProjectionType() == kPT_Perspective)
         {
            frustum.ExtractPlanes(pCamera->GetViewProjectionMatrix());
            bHaveFrustum = true;
         }
         else
         {
            // Don't frustum-cull orthographically projected stuff like UI
            bHaveFrustum = false;
         }
      }

      tSceneEntityList entities;

      if (bHaveFrustum)
      {
         m_layers[i].Cull(frustum, &entities);
      }
      else
      {
         m_layers[i].GetAll(&entities);
      }

      if (!entities.empty())
      {
         std::for_each(entities.begin(), entities.end(), cRenderEntity(pRenderDevice));
         std::for_each(entities.begin(), entities.end(), CTInterfaceMethod(&::IUnknown::Release));
      }
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

tResult cScene::Query(const cRay & ray, ISceneEntityEnum * * ppEnum)
{
   tSceneEntityList entities;
   for (int i = 0; i < _countof(m_layers); i++)
   {
      m_layers[i].Query(ray, &entities);
   }

   tResult result = S_FALSE;

   if (!entities.empty())
   {
      *ppEnum = SceneEntityEnumCreate(entities);
      result = S_OK;
   }

   std::for_each(entities.begin(), entities.end(), CTInterfaceMethod(&::IUnknown::Release));

   return result;
}

///////////////////////////////////////

cScene::cInputListener::cInputListener(cScene * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

bool cScene::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   Assert(m_pOuter != NULL);

   for (int i = _countof(m_pOuter->m_layers) - 1; i >= 0; i--)
   {
      if (m_pOuter->m_layers[i].HandleInputEvent(pEvent))
      {
         return true;
      }
   }

   return false;
}

///////////////////////////////////////

void SceneCreate()
{
   cAutoIPtr<IScene>(new cScene);
}

///////////////////////////////////////////////////////////////////////////////
