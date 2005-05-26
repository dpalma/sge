///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitymanager.h"
#include "model.h"

#include "meshapi.h"
#include "sceneapi.h"

#include "resourceapi.h"
#include "vec2.h"
#include "vec3.h"

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

///////////////////////////////////////

cEntityManager::cEntityManager()
 : cGlobalObject<IMPLEMENTS(IEntityManager)>("EntityManager"),
   m_nextId(0),
   m_pRenderDevice(NULL),
   m_pTerrainLocatorHack(NULL)
{
}

///////////////////////////////////////

cEntityManager::~cEntityManager()
{
}

///////////////////////////////////////

tResult cEntityManager::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::Term()
{
   return S_OK;
}

///////////////////////////////////////

void cEntityManager::SetRenderDeviceHack(IRenderDevice * pRenderDevice)
{
   m_pRenderDevice = pRenderDevice;
}

///////////////////////////////////////

void cEntityManager::SetTerrainLocatorHack(cTerrainLocatorHack * pTerrainLocatorHack)
{
   m_pTerrainLocatorHack = pTerrainLocatorHack;
}

///////////////////////////////////////

tResult cEntityManager::SpawnEntity(const char * pszMesh, float x, float z)
{
   tVec3 location(0,0,0);

   if (m_pTerrainLocatorHack != NULL)
   {
      m_pTerrainLocatorHack->Locate(x, z, &location.x, &location.y, &location.z);
   }

   cAutoIPtr<IMesh> pMesh;
   UseGlobal(ResourceManager);
   if (pResourceManager->LoadUncached(tResKey(pszMesh, kRC_Mesh), m_pRenderDevice, (void**)&pMesh, NULL) != S_OK)
   {
      DebugMsg1("Error loading mesh \"%s\"\n", pszMesh);
      return E_FAIL;
   }

   cAutoIPtr<ISceneEntity> pEntity = SceneEntityCreate(pMesh);
   pEntity->SetLocalTranslation(location);

   UseGlobal(Scene);
   pScene->AddEntity(kSL_Object, pEntity);

   // HACK: add a cModel-based entity too to test new code
   {
      cAutoIPtr<ISceneEntity> pModelTestEntity = SceneEntityCreate(pszMesh);
      pModelTestEntity->SetLocalTranslation(location);
      pScene->AddEntity(kSL_Object, pModelTestEntity);
   }

   return S_OK;
}

///////////////////////////////////////

void EntityManagerCreate()
{
   cAutoIPtr<IEntityManager> p(new cEntityManager);
}

///////////////////////////////////////////////////////////////////////////////
