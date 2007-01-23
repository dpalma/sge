///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitymanager.h"

#include "engine/cameraapi.h"
#include "engine/engineapi.h"
#include "render/renderapi.h"
#include "engine/terrainapi.h"
#include "platform/keys.h"

#include "tech/color.h"
#include "tech/multivar.h"
#include "tech/ray.h"
#include "tech/readwriteutils.h"
#include "tech/resourceapi.h"

#include <tinyxml.h>

#include <algorithm>

#include "tech/dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' : used in base member initializer list

///////////////////////////////////////////////////////////////////////////////

#define IsFlagSet(f, b) (((f)&(b))==(b))

extern tResult EntityCreate(const tChar * pszTypeName, tEntityId id, IEntity * * ppEntity);
extern void RegisterBuiltInComponents();


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

///////////////////////////////////////

cEntityManager::cEntityManager()
 : m_nextId(0)
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

////////////////////////////////////////

BEGIN_CONSTRAINTS(cEntityManager)
   AFTER_GUID(IID_IScheduler)
END_CONSTRAINTS()

///////////////////////////////////////

tResult cEntityManager::Init()
{
   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RegisterSaveLoadParticipant(SAVELOADID_EntityManager,
      g_entityManagerVer, static_cast<ISaveLoadParticipant*>(this));

   UseGlobal(Scheduler);
   pScheduler->AddFrameTask(&m_updateTask, 0, 1, 0);

   UseGlobal(Input);
   pInput->AddInputListener(&m_inputListener);

   RegisterBuiltInComponents();

   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::Term()
{
   m_updateTask.RemoveAll();

   UseGlobal(Scheduler);
   pScheduler->RemoveFrameTask(&m_updateTask);

   UseGlobal(Input);
   pInput->RemoveInputListener(&m_inputListener);

   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RevokeSaveLoadParticipant(SAVELOADID_EntityManager, g_entityManagerVer);

   DeselectAll();
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
   if (pszEntity == NULL)
   {
      return E_POINTER;
   }

   const TiXmlDocument * pTiXmlDoc = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszEntity, kRT_TiXml, NULL, (void**)&pTiXmlDoc) == S_OK)
   {
      const TiXmlElement * pTiXmlElement = pTiXmlDoc->FirstChildElement();
      if ((pTiXmlElement != NULL) && (_stricmp(pTiXmlElement->Value(), "entity") == 0))
      {
         WarnMsgIf1(pTiXmlElement->NextSiblingElement() != NULL,
            "There should be only one entity definition per file (%s)\n", pszEntity);

         uint oldNextId = m_nextId;
         uint entityId = ++m_nextId;

         tResult result = E_FAIL;
         cAutoIPtr<IEntity> pEntity;
         if ((result = EntityCreate(pszEntity, entityId, &pEntity)) != S_OK)
         {
            m_nextId = oldNextId;
            return result;
         }

         for (const TiXmlElement * pTiXmlChild = pTiXmlElement->FirstChildElement();
            pTiXmlChild != NULL; pTiXmlChild = pTiXmlChild->NextSiblingElement())
         {
            Assert(pTiXmlChild->Type() == TiXmlNode::ELEMENT);

            cAutoIPtr<IEntityComponent> pComponent;
            if (CreateComponent(pTiXmlChild, pEntity, &pComponent) == S_OK)
            {
               cAutoIPtr<IEntityPositionComponent> pPosition;
               if (pComponent->QueryInterface(IID_IEntityPositionComponent, (void**)&pPosition) == S_OK)
               {
                  pPosition->SetPosition(position);
               }
            }
            else
            {
               WarnMsgIf1(pTiXmlChild->Value() != NULL, "Failed to create entity component \"%s\"\n", pTiXmlChild->Value());
            }
         }

         m_entities.push_back(CTAddRef(pEntity));
         if (pEntityId != NULL)
         {
            *pEntityId = entityId;
         }

         RegisterEntityUpdatables(pEntity);

         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cEntityManager::RemoveEntity(tEntityId entityId)
{
   bool bFound = false;
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));

      if (pEntity->GetId() == entityId)
      {
         bFound = true;

         RevokeEntityUpdatables(pEntity);

         size_t nErasedFromSelected = m_selected.erase(pEntity);
         while (nErasedFromSelected-- > 0)
         {
            pEntity->Release();
         }

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
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      if (CTIsSameObject(pEntity, *iter))
      {
         bFound = true;

         RevokeEntityUpdatables(*iter);

         (*iter)->Release();
         m_entities.erase(iter);

         // TODO: return entity's id to a pool?

         break;
      }
   }

   size_t nErasedFromSelected = m_selected.erase(pEntity);
   while (nErasedFromSelected-- > 0)
   {
      pEntity->Release();
   }

   return bFound ? S_OK : S_FALSE;
}

///////////////////////////////////////

void cEntityManager::RemoveAll()
{
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      RevokeEntityUpdatables(*iter);
      (*iter)->Release();
   }
   m_entities.clear();
}

///////////////////////////////////////

void cEntityManager::RenderAll()
{
   UseGlobal(Renderer);

   tEntityList::iterator iter = m_entities.begin(), end = m_entities.end();
   for (; iter != end; ++iter)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));

      bool bPopMatrix = false;
      cAutoIPtr<IEntityPositionComponent> pPosition;
      if (pEntity->GetComponent(kECT_Position, IID_IEntityPositionComponent, &pPosition) == S_OK)
      {
         pRenderer->PushMatrix(pPosition->GetWorldTransform().m);
         bPopMatrix = true;
      }

      cAutoIPtr<IEntityRenderComponent> pRender;
      if (pEntity->GetComponent(kECT_Render, IID_IEntityRenderComponent, &pRender) == S_OK)
      {
         bool bSelected = IsSelected(*iter);

         pRender->Render(bSelected ? kERF_Selected : kERF_None);

         if (bSelected)
         {
            tAxisAlignedBox bbox;
            if (pRender->GetBoundingBox(&bbox) == S_OK)
            {
               RenderWireFrameBox(bbox, cColor(1,1,0).GetPointer());
            }
         }
      }

      if (bPopMatrix)
      {
         pRenderer->PopMatrix();
      }
   }
}

///////////////////////////////////////

tResult cEntityManager::RayCast(const cRay & ray, IEntity * * ppEntity) const
{
   tEntityList::const_iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));

      cAutoIPtr<IEntityPositionComponent> pPosition;
      if (pEntity->GetComponent(kECT_Position, IID_IEntityPositionComponent, &pPosition) == S_OK)
      {
         tVec3 position;
         if (pPosition->GetPosition(&position) == S_OK)
         {
            cAutoIPtr<IEntityRenderComponent> pRender;
            if (pEntity->GetComponent(kECT_Render, IID_IEntityRenderComponent, &pRender) == S_OK)
            {
               tAxisAlignedBox bbox;
               if (pRender->GetBoundingBox(&bbox) == S_OK)
               {
                  bbox.Offset(position);

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
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cEntityManager::Select(IEntity * pEntity)
{
   if (pEntity == NULL)
   {
      return E_POINTER;
   }

   std::pair<tEntitySet::iterator, bool> result = m_selected.insert(pEntity);
   if (result.second)
   {
      pEntity->AddRef();
      ForEachConnection(&IEntityManagerListener::OnEntitySelectionChange);
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cEntityManager::SelectBoxed(const tAxisAlignedBox & box)
{
   int nSelected = 0;
   tEntityList::const_iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));

      cAutoIPtr<IEntityPositionComponent> pPosition;
      if (pEntity->GetComponent(kECT_Position, IID_IEntityPositionComponent, &pPosition) == S_OK)
      {
         tVec3 position;
         if (pPosition->GetPosition(&position) == S_OK)
         {
            cAutoIPtr<IEntityRenderComponent> pRender;
            if (pEntity->GetComponent(kECT_Render, IID_IEntityRenderComponent, &pRender) == S_OK)
            {
               tAxisAlignedBox bbox;
               if (pRender->GetBoundingBox(&bbox) == S_OK)
               {
                  bbox.Offset(position);
                  if (bbox.Intersects(box))
                  {
                     m_selected.insert(CTAddRef(pEntity));
                     nSelected++;
                  }
               }
            }
         }
      }
   }

   if (nSelected == 0)
   {
      return S_FALSE;
   }

   ForEachConnection(&IEntityManagerListener::OnEntitySelectionChange);
   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::DeselectAll()
{
   if (m_selected.empty())
   {
      return S_FALSE;
   }
   std::for_each(m_selected.begin(), m_selected.end(), CTInterfaceMethod(&IEntity::Release));
   m_selected.clear();
   ForEachConnection(&IEntityManagerListener::OnEntitySelectionChange);
   return S_OK;
}

///////////////////////////////////////

uint cEntityManager::GetSelectedCount() const
{
   return m_selected.size();
}

///////////////////////////////////////

tResult cEntityManager::SetSelected(IEnumEntities * pEnum)
{
   if (pEnum == NULL)
   {
      return E_POINTER;
   }

   std::for_each(m_selected.begin(), m_selected.end(), CTInterfaceMethod(&IEntity::Release));
   m_selected.clear();

   IEntity * pEntities[32];
   ulong count = 0;

   while (SUCCEEDED(pEnum->Next(_countof(pEntities), &pEntities[0], &count)) && (count > 0))
   {
      for (ulong i = 0; i < count; i++)
      {
         std::pair<tEntitySet::iterator, bool> result = m_selected.insert(pEntities[i]);
         if (result.second)
         {
            pEntities[i]->AddRef();
         }

         SafeRelease(pEntities[i]);
      }

      count = 0;
   }

   ForEachConnection(&IEntityManagerListener::OnEntitySelectionChange);

   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::GetSelected(IEnumEntities * * ppEnum) const
{
   if (ppEnum == NULL)
   {
      return E_POINTER;
   }
   if (m_selected.empty())
   {
      return S_FALSE;
   }
   return tEntitySetEnum::Create(m_selected, ppEnum);
}

///////////////////////////////////////

tResult cEntityManager::RegisterComponentFactory(const tChar * pszComponent,
                                                 tEntityComponentFactoryFn pfnFactory,
                                                 void * pUser)
{
   if (pszComponent == NULL || pfnFactory == NULL)
   {
      return E_POINTER;
   }

   std::pair<tComponentFactoryMap::iterator, bool> result = 
      m_componentFactoryMap.insert(std::make_pair(pszComponent, std::make_pair(pfnFactory, pUser)));
   if (result.second)
   {
      return S_OK;
   }

   WarnMsg1("Failed to register entity component factory \"%s\"\n", pszComponent);
   return E_FAIL;
}

///////////////////////////////////////

tResult cEntityManager::RevokeComponentFactory(const tChar * pszComponent)
{
   if (pszComponent == NULL)
   {
      return E_POINTER;
   }
   size_t nErased = m_componentFactoryMap.erase(pszComponent);
   return (nErased == 0) ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cEntityManager::CreateComponent(const TiXmlElement * pTiXmlElement,
                                        IEntity * pEntity, IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

#ifdef _UNICODE
   cMultiVar temp(pTiXmlElement->Value());
   const wchar_t * pszComponent = temp.ToWideString();
#else
   const char * pszComponent = pTiXmlElement->Value();
#endif

   tComponentFactoryMap::iterator f = m_componentFactoryMap.find(pszComponent);
   if (f != m_componentFactoryMap.end())
   {
      const std::pair<tEntityComponentFactoryFn, void*> & p = f->second;
      return (*p.first)(pTiXmlElement, pEntity, p.second, ppComponent);
   }

   return E_FAIL;
}

///////////////////////////////////////

void cEntityManager::RegisterEntityUpdatables(IEntity * pEntity)
{
   if (pEntity == NULL)
   {
      return;
   }

   for (int i = 0; i < kMaxEntityComponentTypes; ++i)
   {
      cAutoIPtr<IUpdatable> pUpdatableComponent;
      if (pEntity->GetComponent(static_cast<eEntityComponentType>(i),
         IID_IUpdatable, &pUpdatableComponent) == S_OK)
      {
         m_updateTask.AddUpdatable(pUpdatableComponent);
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

   for (int i = 0; i < kMaxEntityComponentTypes; ++i)
   {
      cAutoIPtr<IUpdatable> pUpdatableComponent;
      if (pEntity->GetComponent(static_cast<eEntityComponentType>(i),
         IID_IUpdatable, &pUpdatableComponent) == S_OK)
      {
         m_updateTask.RemoveUpdatable(pUpdatableComponent);
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
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      cAutoIPtr<IEntityPositionComponent> pPosition;
      if ((*iter)->GetComponent(kECT_Position, IID_IEntityPositionComponent, &pPosition) != S_OK)
      {
         continue;
      }

      cStr typeName;
      tVec3 position;
      if ((*iter)->GetTypeName(&typeName) == S_OK && !typeName.empty() && pPosition->GetPosition(&position) == S_OK)
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

bool cEntityManager::IsSelected(IEntity * pEntity) const
{
   return (m_selected.find(pEntity) != m_selected.end());
}

///////////////////////////////////////

cEntityManager::cUpdateTask::cUpdateTask()
 : m_lastTime(0)
{
}

///////////////////////////////////////

tResult cEntityManager::cUpdateTask::Execute(double time)
{
   double elapsed = fabs(time - m_lastTime);
   tUpdatableList::iterator iter = m_updatables.begin(), end = m_updatables.end();
   for (; iter != end; ++iter)
   {
      (*iter)->Update(elapsed);
   }
   m_lastTime = time;
   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::cUpdateTask::AddUpdatable(IUpdatable * pUpdatable)
{
   return add_interface(m_updatables, pUpdatable) ? S_OK : E_FAIL;
}

///////////////////////////////////////

tResult cEntityManager::cUpdateTask::RemoveUpdatable(IUpdatable * pUpdatable)
{
   return remove_interface(m_updatables, pUpdatable) ? S_OK : E_FAIL;
}

///////////////////////////////////////

void cEntityManager::cUpdateTask::RemoveAll()
{
   tUpdatableList::iterator iter = m_updatables.begin(), end = m_updatables.end();
   for (; iter != end; ++iter)
   {
      (*iter)->Release();
   }
   m_updatables.clear();
}

///////////////////////////////////////

bool cEntityManager::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   if (pEvent->down && pEvent->key == kMouseLeft)
   {
      UseGlobal(Renderer);

      cAutoIPtr<IRenderCamera> pCamera;
      cRay pickRay;
      if (pRenderer->GetCamera(&pCamera) == S_OK
         && pCamera->GenerateScreenPickRay(pEvent->point.x, pEvent->point.y, &pickRay) == S_OK)
      {
         cAutoIPtr<IEntity> pEntity;
         UseGlobal(EntityManager);
         if (pEntityManager->RayCast(pickRay, &pEntity) == S_OK)
         {
            pEntityManager->Select(pEntity);
         }
         else
         {
            pEntityManager->DeselectAll();
         }
      }
   }

   return false;
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
