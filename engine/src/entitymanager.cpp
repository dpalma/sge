///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitymanager.h"

#include "renderapi.h"
#include "terrainapi.h"

#include "color.h"
#include "ray.h"
#include "resourceapi.h"

#include <algorithm>
#include <cfloat>

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

cModelEntity::cModelEntity(uint id, const tChar * pszModel, const tVec3 & position)
 : m_model(pszModel)
 , m_pModel(NULL)
 , m_id(id)
 , m_flags(kEF_None)
 , m_position(position)
 , m_bUpdateWorldTransform(true)
{
}

///////////////////////////////////////

cModelEntity::~cModelEntity()
{
}

///////////////////////////////////////

uint cModelEntity::GetId() const
{
   return m_id;
}

///////////////////////////////////////

uint cModelEntity::GetFlags() const
{
   return m_flags;
}

///////////////////////////////////////

uint cModelEntity::SetFlags(uint flags, uint mask)
{
   m_flags = (m_flags & ~mask) | (flags & mask);
   return m_flags;
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

const tAxisAlignedBox & cModelEntity::GetBoundingBox() const
{
   return m_bbox;
}

///////////////////////////////////////

static void CalculateBBox(const tModelVertices & vertices, tAxisAlignedBox * pBBox)
{
   tVec3 mins(FLT_MAX, FLT_MAX, FLT_MAX), maxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);
   tModelVertices::const_iterator iter = vertices.begin();
   for (; iter != vertices.end(); iter++)
   {
      if (iter->pos.x < mins.x)
      {
         mins.x = iter->pos.x;
      }
      if (iter->pos.y < mins.y)
      {
         mins.y = iter->pos.y;
      }
      if (iter->pos.z < mins.z)
      {
         mins.z = iter->pos.z;
      }
      if (iter->pos.x > maxs.x)
      {
         maxs.x = iter->pos.x;
      }
      if (iter->pos.y > maxs.y)
      {
         maxs.y = iter->pos.y;
      }
      if (iter->pos.z > maxs.z)
      {
         maxs.z = iter->pos.z;
      }
   }
   *pBBox = tAxisAlignedBox(mins, maxs);
}

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
         if (m_pModel->GetSkeleton() != NULL || m_pModel->GetSkeleton()->IsAnimated())
         {
            ModelAnimationControllerCreate(m_pModel->GetSkeleton(), &m_pAnimController);
         }
         CalculateBBox(m_pModel->GetVertices(), &m_bbox);
      }
   }

   if (!!m_pAnimController)
   {
      m_pAnimController->Advance(elapsedTime);
      m_pModel->ApplyJointMatrices(m_pAnimController->GetBlendMatrices(), &m_blendedVerts);
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

   pRenderer->SetIndexFormat(kIF_16Bit);

   tModelMeshes::const_iterator iter = m_pModel->BeginMeshses();
   tModelMeshes::const_iterator end = m_pModel->EndMeshses();
   for (; iter != end; iter++)
   {
      int iMaterial = iter->GetMaterialIndex();
      if (iMaterial >= 0)
      {
         m_pModel->GetMaterial(iMaterial).GlDiffuseAndTexture();
      }
      pRenderer->Render(iter->GetPrimitiveType(), const_cast<uint16*>(iter->GetIndexData()), iter->GetIndexCount());
   }

   RenderWireFrame(m_bbox, cColor(1,1,0));
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

   DeselectAll();

   tEntityList::iterator iter = m_entities.begin();
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
   uint oldNextId = m_nextId;
   cAutoIPtr<IEntity> pEntity(static_cast<IEntity*>(new cModelEntity(m_nextId++, pszMesh, position)));
   if (!pEntity)
   {
      m_nextId = oldNextId;
      return E_OUTOFMEMORY;
   }

   m_entities.push_back(CTAddRef(pEntity));
   return S_OK;
}

///////////////////////////////////////

void cEntityManager::RenderAll()
{
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      glPushMatrix();
      glMultMatrixf((*iter)->GetWorldTransform().m);
      (*iter)->Render();
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

tResult cEntityManager::SelectBoxed(const tAxisAlignedBox & box)
{
   return E_NOTIMPL;
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
