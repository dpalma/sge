///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitymanager.h"
#include "model.h"
#include "sceneapi.h"

#include "resourceapi.h"
#include "vec2.h"
#include "vec3.h"
#include "configapi.h"

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

///////////////////////////////////////

cEntityManager::cEntityManager()
 : m_nextId(0),
   m_pRenderDevice(NULL),
   m_pTerrainLocatorHack(NULL)
{
   RegisterGlobalObject(IID_IEntityManager, static_cast<IGlobalObject*>(this));
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

   cAutoIPtr<ISceneEntity> pModelTestEntity = SceneEntityCreate(pszMesh);
   pModelTestEntity->SetLocalTranslation(location);
   UseGlobal(Scene);
   pScene->AddEntity(kSL_Object, pModelTestEntity);

   return S_OK;
}

///////////////////////////////////////

void EntityManagerCreate()
{
   cAutoIPtr<IEntityManager> p(new cEntityManager);
}

///////////////////////////////////////////////////////////////////////////////
