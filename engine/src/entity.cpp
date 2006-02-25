///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entity.h"

#include "renderapi.h"

#include "color.h"
#include "globalobj.h"
#include "ray.h"
#include "resourceapi.h"

#include <cfloat>

#include <GL/glew.h>

#include "dbgalloc.h" // must be last header

#define IsFlagSet(f, b) (((f)&(b))==(b))

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

tResult ModelEntityCreate(tEntityId id, const tChar * pszModel, const tVec3 & position, IEntity * * ppEntity)
{
   if (pszModel == NULL || ppEntity == NULL)
   {
      return E_POINTER;
   }

   cModelEntity * pModelEntity = new cModelEntity(id, pszModel, position);
   if (pModelEntity == NULL)
   {
      return E_OUTOFMEMORY;
   }

   *ppEntity = static_cast<IEntity*>(pModelEntity);
   return S_OK;
}

///////////////////////////////////////

cModelEntity::cModelEntity(tEntityId id, const tChar * pszModel, const tVec3 & position)
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

tEntityId cModelEntity::GetId() const
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

tResult cModelEntity::GetModel(cStr * pModel) const
{
   if (pModel == NULL)
   {
      return E_POINTER;
   }
   if (m_model.empty())
   {
      return S_FALSE;
   }
   *pModel = m_model;
   return S_OK;
}

///////////////////////////////////////

tResult cModelEntity::GetPosition(tVec3 * pPosition) const
{
   if (pPosition == NULL)
   {
      return E_POINTER;
   }
   *pPosition = m_position;
   return S_OK;
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
}


///////////////////////////////////////////////////////////////////////////////
