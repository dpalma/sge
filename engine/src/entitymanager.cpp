///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitymanager.h"

#include "cameraapi.h"
#include "engineapi.h"
#include "readwriteutils.h"
#include "renderapi.h"
#include "terrainapi.h"

#include "color.h"
#include "keys.h"
#include "ray.h"
#include "resourceapi.h"

#include <tinyxml.h>

#include <GL/glew.h>

#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#define IsFlagSet(f, b) (((f)&(b))==(b))

extern tResult ModelEntityCreate(tEntityId id, const tChar * pszModel, const tVec3 & position, IEntity * * ppEntity);


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

static const int g_entityManagerVer = 1;

///////////////////////////////////////

tResult cEntityManager::Init()
{
   UseGlobal(Sim);
   pSim->Connect(static_cast<ISimClient*>(this));

   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RegisterSaveLoadParticipant(SAVELOADID_EntityManager,
      g_entityManagerVer, static_cast<ISaveLoadParticipant*>(this));

   UseGlobal(Input);
   pInput->AddInputListener(&m_inputListener);

   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::Term()
{
   UseGlobal(Input);
   pInput->RemoveInputListener(&m_inputListener);

   UseGlobal(Sim);
   pSim->Disconnect(static_cast<ISimClient*>(this));

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

tResult cEntityManager::SpawnEntity(const tChar * pszEntity, const tVec3 & position)
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
         const char * pszModel = pTiXmlElement->Attribute("model");

         WarnMsgIf1(pTiXmlElement->NextSiblingElement() != NULL,
            "There should be only one entity definition per file (%s)\n", pszEntity);
      }
   }

   uint oldNextId = m_nextId;
   cAutoIPtr<IEntity> pEntity;
   if (ModelEntityCreate(m_nextId++, pszEntity, position, &pEntity) != S_OK)
   {
      m_nextId = oldNextId;
      return E_OUTOFMEMORY;
   }

   m_entities.push_back(CTAddRef(pEntity));
   return S_OK;
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
      (*iter)->Release();
   }
   m_entities.clear();
}

///////////////////////////////////////

void cEntityManager::RenderAll()
{
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      uint flags = (*iter)->GetFlags();
      if (IsFlagSet(flags, kEF_Hidden))
      {
         continue;
      }

      glPushMatrix();
      glMultMatrixf((*iter)->GetWorldTransform().m);

      (*iter)->Render();

      if (IsSelected(*iter))
      {
         RenderWireFrame((*iter)->GetBoundingBox(), cColor(1,1,0));
      }

      glPopMatrix();
   }
}

///////////////////////////////////////

tResult cEntityManager::RayCast(const cRay & ray, IEntity * * ppEntity) const
{
   tEntityList::const_iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));
      const tMatrix4 & worldTransform = pEntity->GetWorldTransform();
      tVec3 position(worldTransform.m[12], worldTransform.m[13], worldTransform.m[14]);
      tAxisAlignedBox bbox(pEntity->GetBoundingBox());
      bbox.Offset(position);
      if (ray.IntersectsAxisAlignedBox(bbox))
      {
         *ppEntity = CTAddRef(pEntity);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cEntityManager::BoxCast(const tAxisAlignedBox & box, IEntityEnum * * ppEnum) const
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
      const tMatrix4 & worldTransform = pEntity->GetWorldTransform();
      tVec3 position(worldTransform.m[12], worldTransform.m[13], worldTransform.m[14]);
      tAxisAlignedBox bbox(pEntity->GetBoundingBox());
      bbox.Offset(position);
      if (bbox.Intersects(box))
      {
         m_selected.insert(CTAddRef(pEntity));
         nSelected++;
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

tResult cEntityManager::GetSelected(IEntityEnum * * ppEnum) const
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

void cEntityManager::OnSimFrame(double elapsedTime)
{
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      uint flags = (*iter)->GetFlags();
      if (IsFlagSet(flags, kEF_Disabled))
      {
         continue;
      }
      (*iter)->Update(elapsedTime);
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
      cStr model;
      tVec3 position;
      if ((*iter)->GetModel(&model) == S_OK
         && (*iter)->GetPosition(&position) == S_OK)
      {
         if (pWriter->Write(model) != S_OK
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

   // version 1:
   // size_t nEntries
   // nEntries of (cStr model, tVec3 position)
   if (version == 1)
   {
      size_t nEntries = 0;
      if (pReader->Read(&nEntries) != S_OK)
      {
         return E_FAIL;
      }
      for (uint i = 0; i < nEntries; i++)
      {
         cStr model;
         tVec3 position;
         if (pReader->Read(&model) != S_OK
            || pReader->Read(&position) != S_OK)
         {
            return E_FAIL;
         }
         SpawnEntity(model.c_str(), position);
      }
   }

   return S_OK;
}

///////////////////////////////////////

bool cEntityManager::IsSelected(IEntity * pEntity) const
{
   return (m_selected.find(pEntity) != m_selected.end());
}

///////////////////////////////////////

bool cEntityManager::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   if (pEvent->down && pEvent->key == kMouseLeft)
   {
      float ndx, ndy;
      ScreenToNormalizedDeviceCoords(pEvent->point.x, pEvent->point.y, &ndx, &ndy);

      cRay pickRay;
      UseGlobal(Camera);
      if (pCamera->GeneratePickRay(ndx, ndy, &pickRay) == S_OK)
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
