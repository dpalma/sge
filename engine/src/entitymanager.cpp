///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitymanager.h"

#include "renderapi.h"
#include "terrainapi.h"

#include "resourceapi.h"

#include <GL/glew.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

const sVertexElement g_modelVert[] =
{
   { kVEU_TexCoord,  kVET_Float2,   0, 0 },
   { kVEU_Normal,    kVET_Float3,   0, 2 * sizeof(float) },
   { kVEU_Position,  kVET_Float3,   0, 5 * sizeof(float) },
   { kVEU_Index,     kVET_Float1,   0, 8 * sizeof(float) },
};

const sVertexElement g_blendedVert[] =
{
   { kVEU_TexCoord,  kVET_Float2,   0, 0 },
   { kVEU_Normal,    kVET_Float3,   0, 2 * sizeof(float) },
   { kVEU_Position,  kVET_Float3,   0, 5 * sizeof(float) },
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelEntity
//

///////////////////////////////////////

cModelEntity::cModelEntity(const tChar * pszModel, const tVec3 & position)
 : m_model(pszModel)
 , m_pModel(NULL)
 , m_animationTime(0)
 , m_animationLength(0)
 , m_position(position)
 , m_bUpdateWorldTransform(true)
{
}

///////////////////////////////////////

cModelEntity::~cModelEntity()
{
}

///////////////////////////////////////

const tMatrix4 & cModelEntity::GetWorldTransform() const
{
   if (m_bUpdateWorldTransform)
   {
      m_bUpdateWorldTransform = false;
      MatrixTranslate(m_position.x, m_position.y, m_position.z, &m_worldTransform);
   }
   return m_worldTransform;
}

///////////////////////////////////////

void cModelEntity::Update(double elapsedTime)
{
   // TODO: When the resource manager is fast enough, this if test should be removed
   // to get a new cModel pointer whenever the resource changes, e.g., by re-saving
   // from a 3D modelling program (3DS Max, or MilkShape, or something).
#if 1
   if (m_pModel == NULL)
#endif
   {
      UseGlobal(ResourceManager);
      cModel * pModel = NULL;
      if (pResourceManager->Load(m_model.c_str(), kRT_Model, NULL, (void**)&pModel) != S_OK)
      {
         m_pModel = NULL;
         return;
      }

      AssertMsg(pModel != NULL, "Should have returned by now if ResourceManager->Load failed\n");

      if (pModel != m_pModel)
      {
         m_pModel = pModel;
         m_animationLength = pModel->GetTotalAnimationLength();
      }
   }

   if (m_pModel->IsAnimated())
   {
      m_animationTime += elapsedTime;
      while (m_animationTime > m_animationLength)
      {
         m_animationTime -= m_animationLength;
      }

      m_pModel->InterpolateJointMatrices(m_animationTime, &m_blendMatrices);
      m_pModel->ApplyJointMatrices(m_blendMatrices, &m_blendedVerts);
   }
}

///////////////////////////////////////

void cModelEntity::Render()
{
   // TODO: When the resource manager is fast enough, this code should be used to
   // get a new cModel pointer in case the resource was changed, e.g., by re-saving
   // from a 3D modelling program (3DS Max, or MilkShape, or something).
#if 0
   UseGlobal(ResourceManager);
   cModel * pModel = NULL;
   if (pResourceManager->Load(m_model.c_str(), kRT_Model, NULL, (void**)&pModel) != S_OK
      || (pModel != m_pModel))
   {
      m_pModel = NULL;
      return;
   }
#else
   if (m_pModel == NULL)
   {
      return;
   }
#endif

   UseGlobal(Renderer);

   glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
   glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

   if (!m_blendedVerts.empty())
   {
      pRenderer->SetVertexFormat(g_blendedVert, _countof(g_blendedVert));
      pRenderer->SubmitVertices(&m_blendedVerts[0], m_blendedVerts.size());
   }
   else
   {
      pRenderer->SetVertexFormat(g_modelVert, _countof(g_modelVert));
      const tModelVertices & verts = m_pModel->GetVertices();
      pRenderer->SubmitVertices(const_cast<sModelVertex *>(&verts[0]), verts.size());
   }

   tModelMeshes::const_iterator iter = m_pModel->BeginMeshses();
   tModelMeshes::const_iterator end = m_pModel->EndMeshses();
   for (; iter != end; iter++)
   {
      int iMaterial = iter->GetMaterialIndex();
      if (iMaterial >= 0)
      {
         m_pModel->GetMaterial(iMaterial).GlDiffuseAndTexture();
      }

#ifdef _DEBUG
      if (GlValidateIndices(iter->GetIndexData(), iter->GetIndexCount(),
         !m_blendedVerts.empty() ? m_blendedVerts.size() : m_pModel->GetVertices().size()))
      {
         glDrawElements(iter->GetGlPrimitive(), iter->GetIndexCount(), GL_UNSIGNED_SHORT, iter->GetIndexData());
      }
#else
      glDrawElements(iter->GetGlPrimitive(), iter->GetIndexCount(), GL_UNSIGNED_SHORT, iter->GetIndexData());
#endif
   }

   glPopClientAttrib();
   glPopAttrib();
}


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

   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::Term()
{
   UseGlobal(Sim);
   pSim->Disconnect(static_cast<ISimClient*>(this));

   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RevokeSaveLoadParticipant(SAVELOADID_EntityManager, g_entityManagerVer);

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

tResult cEntityManager::SpawnEntity(const tChar * pszMesh, float nx, float nz)
{
   tVec3 location(0,0,0);
   if (m_pTerrainLocatorHack != NULL)
   {
      m_pTerrainLocatorHack->Locate(nx, nz, &location.x, &location.y, &location.z);
   }
   else
   {
      cTerrainSettings terrainSettings;
      UseGlobal(TerrainModel);
      pTerrainModel->GetTerrainSettings(&terrainSettings);
      location.x = nx * terrainSettings.GetTileCountX() * terrainSettings.GetTileSize();
      location.y = 0; // TODO: get real elevation
      location.z = nz * terrainSettings.GetTileCountZ() * terrainSettings.GetTileSize();
   }

   return SpawnEntity(pszMesh, location);
}

///////////////////////////////////////

tResult cEntityManager::SpawnEntity(const tChar * pszMesh, const tVec3 & position)
{
   cAutoIPtr<cModelEntity> pEntity(new cModelEntity(pszMesh, position));
   if (!pEntity)
   {
      return E_NOTIMPL;
   }

   m_entities.push_back(CTAddRef(pEntity));
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
      (*iter)->Render();
      glPopMatrix();
   }
}

///////////////////////////////////////

void cEntityManager::OnSimFrame(double elapsedTime)
{
   tEntities::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      (*iter)->Update(elapsedTime);
   }
}

///////////////////////////////////////

tResult cEntityManager::Save(IWriter * pWriter)
{
   return S_FALSE;
}

///////////////////////////////////////

tResult cEntityManager::Load(IReader * pReader, int version)
{
   return S_FALSE;
}

///////////////////////////////////////

tResult EntityManagerCreate()
{
   cAutoIPtr<IEntityManager> p(static_cast<IEntityManager*>(new cEntityManager));
   return RegisterGlobalObject(IID_IEntityManager, p);
}

///////////////////////////////////////////////////////////////////////////////
