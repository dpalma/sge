///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"
#include "render/renderapi.h"

#include "tech/resourceapi.h"
#include "tech/readwriteapi.h"
#include "tech/globalobj.h"
#include "tech/filespec.h"
#include "tech/techmath.h"

#include <algorithm>
#include <cfloat>

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Model);

#define LocalMsg(msg)            DebugMsgEx2(Model,msg)
#define LocalMsg1(msg,a)         DebugMsgEx3(Model,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx4(Model,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx5(Model,msg,(a),(b),(c))

typedef std::vector< cMatrix4<float> > tMatrices;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelMesh
//

///////////////////////////////////////

cModelMesh::cModelMesh()
{
}

///////////////////////////////////////

cModelMesh::cModelMesh(const cModelMesh & other)
 : m_primitive(other.m_primitive)
 , m_indices(other.m_indices.size())
 , m_materialIndex(other.m_materialIndex)
{
   std::copy(other.m_indices.begin(), other.m_indices.end(), m_indices.begin());
}

///////////////////////////////////////

cModelMesh::cModelMesh(ePrimitiveType primitive, const std::vector<uint16> & indices, int8 materialIndex)
 : m_primitive(primitive)
 , m_indices(indices.size())
 , m_materialIndex(materialIndex)
{
   std::copy(indices.begin(), indices.end(), m_indices.begin());
}

///////////////////////////////////////

cModelMesh::~cModelMesh()
{
}

///////////////////////////////////////

const cModelMesh & cModelMesh::operator =(const cModelMesh & other)
{
   m_primitive = other.m_primitive;
   m_materialIndex = other.m_materialIndex;
   m_indices.resize(other.m_indices.size());
   std::copy(other.m_indices.begin(), other.m_indices.end(), m_indices.begin());
   return *this;
}


///////////////////////////////////////////////////////////////////////////////

bool GlValidateIndices(const uint16 * pIndices, uint nIndices, uint nVertices)
{
   for (uint i = 0; i < nIndices; i++)
   {
      if (pIndices[i] >= nVertices)
      {
         ErrorMsg2("INDEX %d OUTSIDE OF VERTEX ARRAY (size %d)!!!\n", pIndices[i], nVertices);
         return false;
      }
   }

   return true;
}


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
               const tModelMaterials & materials,
               const tModelMeshes & meshes)
 : m_vertices(verts.size())
 , m_materials(materials.size())
 , m_meshes(meshes.size())
{
   std::copy(verts.begin(), verts.end(), m_vertices.begin());
   std::copy(materials.begin(), materials.end(), m_materials.begin());
   std::copy(meshes.begin(), meshes.end(), m_meshes.begin());
}

///////////////////////////////////////

cModel::cModel(const tModelVertices & verts,
               const std::vector<uint16> & indices,
               const std::vector<sModelMesh> & meshes2,
               const tModelMaterials & materials,
               IModelSkeleton * pSkeleton)
 : m_vertices(verts.size())
 , m_indices(indices)
 , m_meshes2(meshes2)
 , m_materials(materials.size())
 , m_pSkeleton(CTAddRef(pSkeleton))
{
   std::copy(verts.begin(), verts.end(), m_vertices.begin());
   std::copy(materials.begin(), materials.end(), m_materials.begin());
}

///////////////////////////////////////

cModel::cModel(const tModelVertices & verts,
               const tModelMaterials & materials,
               const tModelMeshes & meshes,
               IModelSkeleton * pSkeleton)
 : m_vertices(verts.size())
 , m_materials(materials.size())
 , m_meshes(meshes.size())
 , m_pSkeleton(CTAddRef(pSkeleton))
{
   std::copy(verts.begin(), verts.end(), m_vertices.begin());
   std::copy(materials.begin(), materials.end(), m_materials.begin());
   std::copy(meshes.begin(), meshes.end(), m_meshes.begin());
}

///////////////////////////////////////

cModel::~cModel()
{
}

///////////////////////////////////////

tResult cModel::Create(const tModelVertices & verts,
                       const tModelMaterials & materials,
                       const tModelMeshes & meshes,
                       cModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   cModel * pModel = new cModel(verts, materials, meshes);
   if (pModel == NULL)
   {
      return E_OUTOFMEMORY;
   }

   *ppModel = pModel;
   return S_OK;
}

///////////////////////////////////////

tResult cModel::Create(const tModelVertices & verts,
                       const std::vector<uint16> & indices,
                       const std::vector<sModelMesh> & meshes2,
                       const tModelMaterials & materials,
                       IModelSkeleton * pSkeleton,
                       cModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   cModel * pModel = new cModel(verts, indices, meshes2, materials, pSkeleton);
   if (pModel == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pModel->PreApplyJoints();

   *ppModel = pModel;
   return S_OK;
}

///////////////////////////////////////

tResult cModel::Create(const tModelVertices & verts,
                       const tModelMaterials & materials,
                       const tModelMeshes & meshes,
                       IModelSkeleton * pSkeleton,
                       cModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   cModel * pModel = new cModel(verts, materials, meshes, pSkeleton);
   if (pModel == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pModel->PreApplyJoints();

   *ppModel = pModel;
   return S_OK;
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
