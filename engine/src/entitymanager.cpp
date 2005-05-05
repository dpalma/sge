///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitymgr.h"
#include "script.h"
#include "groundtiled.h"

#include "meshapi.h"
#include "sceneapi.h"
#include "scriptvar.h"

#include "resourceapi.h"
#include "vec2.h"

#include "dbgalloc.h" // must be last header

F_DECLARE_INTERFACE(IRenderDevice);
extern IRenderDevice * AccessRenderDevice();

extern cAutoIPtr<cTerrainNode> g_pTerrainRoot;

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(EntitySpawnTest)
{
   if (ScriptArgc() == 3
      && ScriptArgIsString(0)
      && ScriptArgIsNumber(1)
      && ScriptArgIsNumber(2))
   {
      const char * pszMesh = ScriptArgAsString(0);
      float x = ScriptArgAsNumber(1);
      float z = ScriptArgAsNumber(2);

      if (x >= 0 && x <= 1 && z >= 0 && z <= 1)
      {
         if (g_pTerrainRoot != NULL)
         {
            float y = g_pTerrainRoot->GetElevation(Round(x), Round(z));

            tVec2 groundDims = g_pTerrainRoot->GetDimensions();

            x *= groundDims.x;
            z *= groundDims.y;

            UseGlobal(EntityManager);
            pEntityManager->SpawnEntity(pszMesh, tVec3(x,y,z));
         }
      }
      else
      {
         DebugMsg2("EntitySpawnTest arguments %f, %f, out of range\n", x, z);
      }
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

///////////////////////////////////////

cEntityManager::cEntityManager()
 : cGlobalObject<IMPLEMENTS(IEntityManager)>("EntityManager"),
   m_nextId(0)
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

tResult cEntityManager::SpawnEntity(const char * pszMesh, const tVec3 & location)
{
   cAutoIPtr<IMesh> pMesh;
   UseGlobal(ResourceManager);
   if (pResourceManager->LoadUncached(tResKey(pszMesh, kRC_Mesh), AccessRenderDevice(), (void**)&pMesh, NULL) != S_OK)
   {
      DebugMsg1("Error loading mesh \"%s\"\n", pszMesh);
      return E_FAIL;
   }

   cAutoIPtr<ISceneEntity> pEntity = SceneEntityCreate(pMesh);
   pEntity->SetLocalTranslation(location);

   UseGlobal(Scene);
   pScene->AddEntity(kSL_Object, pEntity);

   return S_OK;
}

///////////////////////////////////////

void EntityManagerCreate()
{
   cAutoIPtr<IEntityManager> p(new cEntityManager);
}

///////////////////////////////////////////////////////////////////////////////
