///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Model);

#define LocalMsg(msg)            DebugMsgEx2(Model,msg)
#define LocalMsg1(msg,a)         DebugMsgEx3(Model,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx4(Model,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx5(Model,msg,(a),(b),(c))


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModel
//

///////////////////////////////////////

cModel::cModel()
{
}

///////////////////////////////////////

cModel::cModel(const tModelVertices & verts,
               const std::vector<uint16> & indices,
               const std::vector<sModelMesh> & meshes,
               const tModelMaterials & materials,
               IModelSkeleton * pSkeleton)
 : m_vertices(verts)
 , m_indices(indices)
 , m_meshes(meshes)
 , m_materials(materials)
 , m_pSkeleton(CTAddRef(pSkeleton))
{
}

///////////////////////////////////////

cModel::~cModel()
{
}

///////////////////////////////////////

tResult cModel::Create(const tModelVertices & verts,
                       const std::vector<uint16> & indices,
                       const std::vector<sModelMesh> & meshes,
                       const tModelMaterials & materials,
                       IModelSkeleton * pSkeleton,
                       IModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   cModel * pModel = new cModel(verts, indices, meshes, materials, pSkeleton);
   if (pModel == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pModel->PreApplyJoints();

   *ppModel = static_cast<IModel*>(pModel);
   return S_OK;
}

///////////////////////////////////////

tResult cModel::GetVertices(uint * pnVertices, const sModelVertex * * ppVertices) const
{
   if (pnVertices != NULL)
   {
      *pnVertices = m_vertices.size();
   }
   if (ppVertices == NULL)
   {
      return E_POINTER;
   }
   if (!m_vertices.empty())
   {
      *ppVertices = &m_vertices[0];
      return S_OK;
   }
   else
   {
      *ppVertices = NULL;
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cModel::GetIndices(uint * pnIndices, const uint16 * * ppIndices) const
{
   if (pnIndices != NULL)
   {
      *pnIndices = m_indices.size();
   }
   if (ppIndices == NULL)
   {
      return E_POINTER;
   }
   if (!m_indices.empty())
   {
      *ppIndices = &m_indices[0];
      return S_OK;
   }
   else
   {
      *ppIndices = NULL;
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cModel::GetMaterialCount(uint * pnMaterials) const
{
   if (pnMaterials == NULL)
   {
      return E_POINTER;
   }
   *pnMaterials = m_materials.size();
   return S_OK;
}

///////////////////////////////////////

tResult cModel::GetMaterial(uint index, sModelMaterial * pModelMaterial) const
{
   if (index >= m_materials.size())
   {
      return E_INVALIDARG;
   }
   if (pModelMaterial == NULL)
   {
      return E_POINTER;
   }
   *pModelMaterial = m_materials[index];
   return S_OK;
}

///////////////////////////////////////

const sModelMaterial * cModel::AccessMaterial(uint index) const
{
   if (index < m_materials.size())
   {
      return &m_materials[index];
   }
   return NULL;
}

///////////////////////////////////////

tResult cModel::GetMeshes(uint * pnMeshes, const sModelMesh * * ppMeshes) const
{
   if (pnMeshes != NULL)
   {
      *pnMeshes = m_meshes.size();
   }
   if (ppMeshes == NULL)
   {
      return E_POINTER;
   }
   if (!m_meshes.empty())
   {
      *ppMeshes = &m_meshes[0];
      return S_OK;
   }
   else
   {
      *ppMeshes = NULL;
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cModel::GetSkeleton(IModelSkeleton * * ppSkeleton)
{
   return m_pSkeleton.GetPointer(ppSkeleton);
}

///////////////////////////////////////
// TODO: How does this work for more than one joint per vertex with blend weights?
// (Answer: I don't think you can pre-apply like this.)

void cModel::PreApplyJoints()
{
   if (!m_pSkeleton)
   {
      return;
   }

   size_t nJoints = 0;
   if (m_pSkeleton->GetJointCount(&nJoints) != S_OK || nJoints == 0)
   {
      return;
   }

   std::vector<tMatrix34> bindMatrices(nJoints);
   m_pSkeleton->GetBindMatrices(bindMatrices.size(), &bindMatrices[0]);

   for (tModelVertices::iterator iter = m_vertices.begin(); iter != m_vertices.end(); iter++)
   {
      int index = FloatToInt(iter->bone);
      if (index < 0)
      {
         continue;
      }

      tVec3 transformedNormal;
      bindMatrices[index].Transform(iter->normal, &transformedNormal);
      iter->normal = transformedNormal;

      tVec3 transformedPosition;
      bindMatrices[index].Transform(iter->pos, &transformedPosition);
      iter->pos = transformedPosition;
   }
}

///////////////////////////////////////////////////////////////////////////////
