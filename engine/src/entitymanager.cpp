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

#include <GL/glew.h>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

///////////////////////////////////////

cEntityManager::cEntityManager()
 : m_nextId(0)
 , m_pTerrainLocatorHack(NULL)
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
   tEntities::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      (*iter)->Release();
   }
   m_entities.clear();

   return S_OK;
}

///////////////////////////////////////

void cEntityManager::SetTerrainLocatorHack(cTerrainLocatorHack * pTerrainLocatorHack)
{
   m_pTerrainLocatorHack = pTerrainLocatorHack;
}

///////////////////////////////////////

tResult cEntityManager::SpawnEntity(const tChar * pszMesh, float x, float z)
{
   tVec3 location(0,0,0);
   if (m_pTerrainLocatorHack != NULL)
   {
      m_pTerrainLocatorHack->Locate(x, z, &location.x, &location.y, &location.z);
   }

   cAutoIPtr<ISceneEntity> pModelTestEntity = SceneEntityCreate(pszMesh);
   pModelTestEntity->SetLocalTranslation(location);

   m_entities.push_back(CTAddRef(pModelTestEntity));

   return S_OK;
}

///////////////////////////////////////

void cEntityManager::RenderAll()
{
   tEntities::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      glPushMatrix();
      glMultMatrixf((*iter)->GetWorldTransform().m);
      (*iter)->Render(NULL);
      glPopMatrix();
   }
}

///////////////////////////////////////

void EntityManagerCreate()
{
   cAutoIPtr<IEntityManager> p(static_cast<IEntityManager*>(new cEntityManager));
   RegisterGlobalObject(IID_IEntityManager, p);
}

///////////////////////////////////////////////////////////////////////////////
