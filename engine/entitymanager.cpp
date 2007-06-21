///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitymanager.h"

#include "engine/cameraapi.h"
#include "engine/engineapi.h"
#include "engine/terrainapi.h"

#include "platform/keys.h"

#include "render/renderapi.h"

#include "tech/color.h"
#include "tech/comenumutil.h"
#include "tech/multivar.h"
#include "tech/point3.inl"
#include "tech/ray.inl"
#include "tech/readwriteutils.h"
#include "tech/resourceapi.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <tinyxml.h>

#include <boost/bind.hpp>
#define BOOST_MEM_FN_ENABLE_STDCALL
#include <boost/mem_fn.hpp>

#include <algorithm>

#include "tech/dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' : used in base member initializer list

using namespace std;
using namespace boost;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

///////////////////////////////////////

cEntityManager::cEntityManager()
{
}

///////////////////////////////////////

cEntityManager::~cEntityManager()
{
}

////////////////////////////////////////

// {DC738464-A124-4dc2-88A5-54619E5D026F}
static const GUID SAVELOADID_EntityManager = 
{ 0xdc738464, 0xa124, 0x4dc2, { 0x88, 0xa5, 0x54, 0x61, 0x9e, 0x5d, 0x2, 0x6f } };

static const int g_entityManagerVer = 2;

///////////////////////////////////////

tResult cEntityManager::Init()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RegisterSaveLoadParticipant(SAVELOADID_EntityManager,
      g_entityManagerVer, static_cast<ISaveLoadParticipant*>(this));

   UseGlobal(Sim);
   pSim->AddSimClient(&m_simClient);

   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::Term()
{
   m_simClient.RemoveAll();

   UseGlobal(Sim);
   pSim->RemoveSimClient(&m_simClient);

   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RevokeSaveLoadParticipant(SAVELOADID_EntityManager, g_entityManagerVer);

   RemoveAll();

   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::AddEntityManagerListener(IEntityManagerListener * pListener)
{
   return Connect(pListener);
}

///////////////////////////////////////

tResult cEntityManager::RemoveEntityManagerListener(IEntityManagerListener * pListener)
{
   return Disconnect(pListener);
}

///////////////////////////////////////

tResult cEntityManager::SpawnEntity(const tChar * pszEntity, const tVec3 & position, tEntityId * pEntityId)
{
   cAutoIPtr<IEntity> pEntity;
   UseGlobal(EntityFactory);
   if (pEntityFactory->CreateEntity(pszEntity, &pEntity) == S_OK)
   {
      cAutoIPtr<IEntityPositionComponent> pPosition;
      if (pEntity->GetComponent(IID_IEntityPositionComponent, &pPosition) == S_OK)
      {
         pPosition->SetPosition(position.v);
      }

      if (pEntityId != NULL)
      {
         *pEntityId = pEntity->GetId();
      }

      return AddEntity(pEntity);
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cEntityManager::AddEntity(IEntity * pEntity)
{
   if (pEntity == NULL)
   {
      return E_POINTER;
   }

   m_entities.push_back(CTAddRef(pEntity));

   RegisterEntityUpdatables(pEntity);

   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::RemoveEntity(tEntityId entityId)
{
   bool bFound = false;
   tEntityList::iterator iter = m_entities.begin(), end = m_entities.end();
   for (; iter != end; ++iter)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));

      if (pEntity->GetId() == entityId)
      {
         bFound = true;

         ForEachConnection(bind(&IEntityManagerListener::OnRemoveEntity, _1, pEntity));

         RevokeEntityUpdatables(pEntity);

         pEntity->Release();
         m_entities.erase(iter);

         // TODO: return entity's id to a pool?

         break;
      }
   }

   return bFound ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cEntityManager::RemoveEntity(IEntity * pEntity)
{
   if (pEntity == NULL)
   {
      return E_POINTER;
   }

   bool bFound = false;
   tEntityList::iterator f = find_if(m_entities.begin(), m_entities.end(), bind1st(ptr_fun(CTIsSameObject), pEntity));
   if (f != m_entities.end())
   {
      bFound = true;

      ForEachConnection(bind(&IEntityManagerListener::OnRemoveEntity, _1, *f));

      RevokeEntityUpdatables(*f);

      (*f)->Release();
      m_entities.erase(f);

      // TODO: return entity's id to a pool?
   }

   return bFound ? S_OK : S_FALSE;
}

///////////////////////////////////////

void cEntityManager::RemoveAll()
{
   tEntityList::iterator iter = m_entities.begin(), end = m_entities.end();
   for (; iter != end; ++iter)
   {
      ForEachConnection(bind(&IEntityManagerListener::OnRemoveEntity, _1, *iter));
      RevokeEntityUpdatables(*iter);
      (*iter)->Release();
   }
   m_entities.clear();
}

///////////////////////////////////////

static void RenderRenderableComponent(IEntityComponent * pComponent)
{
   cAutoIPtr<IRenderable> pRenderable;
   if (pComponent->QueryInterface(IID_IRenderable, (void**)&pRenderable) == S_OK)
   {
      pRenderable->Render();
   }
}

static void RenderEntity(IEntity * pEntity)
{
   UseGlobal(Renderer);

   bool bPopMatrix = false;
   cAutoIPtr<IEntityPositionComponent> pPosition;
   if (pEntity->GetComponent(IID_IEntityPositionComponent, &pPosition) == S_OK)
   {
      pRenderer->PushMatrix(pPosition->GetWorldTransform().m);
      bPopMatrix = true;
   }

   cAutoIPtr<IEnumEntityComponents> pEnum;
   if (pEntity->EnumComponents(IID_IRenderable, &pEnum) == S_OK)
   {
      ForEach<IEnumEntityComponents, IEntityComponent>(pEnum, RenderRenderableComponent);
   }

   if (bPopMatrix)
   {
      pRenderer->PopMatrix();
   }
}

void cEntityManager::RenderAll()
{
   for_each(m_entities.begin(), m_entities.end(), &RenderEntity);
}

///////////////////////////////////////

tResult cEntityManager::RayCast(const cRay<float> & ray, IEntity * * ppEntity) const
{
   tEntityList::const_iterator iter = m_entities.begin(), end = m_entities.end();
   for (; iter != end; ++iter)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));

      cAutoIPtr<IEntityPositionComponent> pPosition;
      if (pEntity->GetComponent(IID_IEntityPositionComponent, &pPosition) == S_OK)
      {
         tVec3 position;
         if (pPosition->GetPosition(position.v) == S_OK)
         {
            cAutoIPtr<IEntityRenderComponent> pRender;
            if (pEntity->GetComponent(IID_IEntityRenderComponent, &pRender) == S_OK)
            {
               tAxisAlignedBox bbox;
               if (pRender->GetBoundingBox(&bbox) == S_OK)
               {
                  bbox.Offset(position.v);

                  if (ray.IntersectsAxisAlignedBox(bbox))
                  {
                     *ppEntity = CTAddRef(pEntity);
                     return S_OK;
                  }
               }
            }
         }
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cEntityManager::BoxCast(const tAxisAlignedBox & box, IEnumEntities * * ppEnum) const
{
   tEntityList boxed;

   tEntityList::const_iterator iter = m_entities.begin(), end = m_entities.end();
   for (; iter != end; ++iter)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));

      cAutoIPtr<IEntityPositionComponent> pPosition;
      if (pEntity->GetComponent(IID_IEntityPositionComponent, &pPosition) == S_OK)
      {
         tVec3 position;
         if (pPosition->GetPosition(position.v) == S_OK)
         {
            cAutoIPtr<IEntityRenderComponent> pRender;
            if (pEntity->GetComponent(IID_IEntityRenderComponent, &pRender) == S_OK)
            {
               tAxisAlignedBox bbox;
               if (pRender->GetBoundingBox(&bbox) == S_OK)
               {
                  bbox.Offset(position.v);
                  if (bbox.Intersects(box))
                  {
                     // Don't AddRef--it will be done in tEntityListEnum::Create
                     boxed.push_back(pEntity);
                  }
               }
            }
         }
      }
   }

   return tEntityListEnum::Create(boxed, ppEnum);
}

///////////////////////////////////////

void cEntityManager::RegisterEntityUpdatables(IEntity * pEntity)
{
   if (pEntity == NULL)
   {
      return;
   }

   cAutoIPtr<IEnumEntityComponents> pEnum;
   if (pEntity->EnumComponents(IID_IUpdatable, &pEnum) == S_OK)
   {
      IEntityComponent * pComponents[32];
      ulong count = 0;

      while (SUCCEEDED(pEnum->Next(_countof(pComponents), &pComponents[0], &count)) && (count > 0))
      {
         for (ulong i = 0; i < count; i++)
         {
            cAutoIPtr<IUpdatable> pUpdatable;
            if (pComponents[i]->QueryInterface(IID_IUpdatable, (void**)&pUpdatable) == S_OK)
            {
               m_simClient.AddUpdatable(pUpdatable);
            }

            SafeRelease(pComponents[i]);
         }

         count = 0;
      }
   }
}

///////////////////////////////////////

void cEntityManager::RevokeEntityUpdatables(IEntity * pEntity)
{
   if (pEntity == NULL)
   {
      return;
   }

   cAutoIPtr<IEnumEntityComponents> pEnum;
   if (pEntity->EnumComponents(IID_IUpdatable, &pEnum) == S_OK)
   {
      IEntityComponent * pComponents[32];
      ulong count = 0;

      while (SUCCEEDED(pEnum->Next(_countof(pComponents), &pComponents[0], &count)) && (count > 0))
      {
         for (ulong i = 0; i < count; i++)
         {
            cAutoIPtr<IUpdatable> pUpdatable;
            if (pComponents[i]->QueryInterface(IID_IUpdatable, (void**)&pUpdatable) == S_OK)
            {
               m_simClient.RemoveUpdatable(pUpdatable);
            }

            SafeRelease(pComponents[i]);
         }

         count = 0;
      }
   }
}

///////////////////////////////////////

tResult cEntityManager::Save(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }
   if (m_entities.empty())
   {
      return S_FALSE;
   }
   if (pWriter->Write(m_entities.size()) != S_OK)
   {
      return E_FAIL;
   }
   tEntityList::iterator iter = m_entities.begin(), end = m_entities.end();
   for (; iter != end; ++iter)
   {
      cAutoIPtr<IEntityPositionComponent> pPosition;
      if ((*iter)->GetComponent(IID_IEntityPositionComponent, &pPosition) != S_OK)
      {
         continue;
      }

      cStr typeName;
      tVec3 position;
      if ((*iter)->GetTypeName(&typeName) == S_OK && !typeName.empty() && pPosition->GetPosition(position.v) == S_OK)
      {
         if (pWriter->Write(typeName) != S_OK
            || pWriter->Write(position) != S_OK)
         {
            return E_FAIL;
         }
      }
   }
   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::Load(IReader * pReader, int version)
{
   if (pReader == NULL)
   {
      return E_POINTER;
   }

   // version 1 stored model names (the entity system was even more primitive)
   if (version == 1)
   {
      return E_FAIL;
   }
   // version 2:
   // size_t nEntries
   // nEntries of (cStr typeName, tVec3 position)
   else if (version == 2)
   {
      size_t nEntries = 0;
      if (pReader->Read(&nEntries) != S_OK)
      {
         return E_FAIL;
      }
      for (uint i = 0; i < nEntries; i++)
      {
         cStr typeName;
         tVec3 position;
         if (pReader->Read(&typeName) != S_OK
            || pReader->Read(&position) != S_OK)
         {
            return E_FAIL;
         }
         SpawnEntity(typeName.c_str(), position, NULL);
      }
   }

   return S_OK;
}

///////////////////////////////////////

void cEntityManager::Reset()
{
   RemoveAll();
}

///////////////////////////////////////

cEntityManager::cSimClient::cSimClient()
 : m_lastTime(0)
{
}

///////////////////////////////////////

tResult cEntityManager::cSimClient::Execute(double time)
{
   double elapsed = fabs(time - m_lastTime);
   for_each(m_updatables.begin(), m_updatables.end(), bind(&IUpdatable::Update, _1, elapsed));
   m_lastTime = time;
   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::cSimClient::AddUpdatable(IUpdatable * pUpdatable)
{
   return add_interface(m_updatables, pUpdatable) ? S_OK : E_FAIL;
}

///////////////////////////////////////

tResult cEntityManager::cSimClient::RemoveUpdatable(IUpdatable * pUpdatable)
{
   return remove_interface(m_updatables, pUpdatable) ? S_OK : E_FAIL;
}

///////////////////////////////////////

void cEntityManager::cSimClient::RemoveAll()
{
   for_each(m_updatables.begin(), m_updatables.end(), mem_fn(&IUnknown::Release));
   m_updatables.clear();
}

///////////////////////////////////////

tResult EntityManagerCreate()
{
   cAutoIPtr<IEntityManager> p(static_cast<IEntityManager*>(new cEntityManager));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IEntityManager, p);
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

//TEST(RemoveSelectedEntity)
//{
//   cAutoIPtr<cEntityManager> pEntityManager(new cEntityManager);
//
//   cAutoIPtr<IEntity> pEntity;
//
//   UseGlobal(EntityFactory);
//   CHECK_EQUAL(S_OK, pEntityFactory->CreateEntity(&pEntity));
//
//   CHECK_EQUAL(S_OK, pEntityManager->AddEntity(pEntity));
//   CHECK_EQUAL(S_OK, pEntityManager->Select(pEntity));
//   CHECK_EQUAL(S_FALSE, pEntityManager->Select(pEntity));
//   CHECK_EQUAL(1, pEntityManager->GetSelectedCount());
//   CHECK_EQUAL(S_OK, pEntityManager->RemoveEntity(pEntity));
//   CHECK_EQUAL(0, pEntityManager->GetSelectedCount());
//}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
