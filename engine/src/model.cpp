///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"
#include "renderapi.h"

#include "resourceapi.h"
#include "readwriteapi.h"
#include "globalobj.h"
#include "filespec.h"
#include "techmath.h"

#include <algorithm>
#include <cfloat>

#include "dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(Model);

#define LocalMsg(ind,msg)           DebugMsgEx2(Model, "%*s" msg, (ind),"")
#define LocalMsg1(ind,msg,a)        DebugMsgEx3(Model, "%*s" msg, (ind),"",(a))
#define LocalMsg2(ind,msg,a,b)      DebugMsgEx4(Model, "%*s" msg, (ind),"",(a),(b))
#define LocalMsg3(ind,msg,a,b,c)    DebugMsgEx5(Model, "%*s" msg, (ind),"",(a),(b),(c))


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelMaterial
//

///////////////////////////////////////

cModelMaterial::cModelMaterial()
{
}

///////////////////////////////////////

cModelMaterial::cModelMaterial(const cModelMaterial & other)
{
   operator =(other);
}

///////////////////////////////////////
// Common case: diffuse and texture only

cModelMaterial::cModelMaterial(const float diffuse[4], const char * pszTexture)
 : m_texture(pszTexture != NULL ? pszTexture : "")
{
   memcpy(m_diffuse, diffuse, sizeof(m_diffuse));
}

///////////////////////////////////////
// All color components

cModelMaterial::cModelMaterial(const float diffuse[4], const float ambient[4],
                               const float specular[4], const float emissive[4],
                               float shininess, const char * pszTexture)
 : m_shininess(shininess)
 , m_texture(pszTexture != NULL ? pszTexture : "")
{
   memcpy(m_diffuse, diffuse, sizeof(m_diffuse));
   memcpy(m_ambient, ambient, sizeof(m_ambient));
   memcpy(m_specular, specular, sizeof(m_specular));
   memcpy(m_emissive, emissive, sizeof(m_emissive));
}

///////////////////////////////////////

cModelMaterial::~cModelMaterial()
{
}

///////////////////////////////////////

const cModelMaterial & cModelMaterial::operator =(const cModelMaterial & other)
{
   memcpy(m_diffuse, other.m_diffuse, sizeof(m_diffuse));
   memcpy(m_ambient, other.m_ambient, sizeof(m_ambient));
   memcpy(m_specular, other.m_specular, sizeof(m_specular));
   memcpy(m_emissive, other.m_emissive, sizeof(m_emissive));
   m_shininess = other.m_shininess;
   m_texture = other.m_texture;
   return *this;
}

///////////////////////////////////////
// Apply diffuse color (for glEnable(GL_COLOR_MATERIAL)) and texture

void cModelMaterial::GlDiffuseAndTexture() const
{
   UseGlobal(Renderer);
   pRenderer->SetDiffuseColor(m_diffuse);
   pRenderer->SetTexture(0, m_texture.c_str());
}



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

   tMatrices inverses(nJoints);
   m_pSkeleton->GetBindMatrices(inverses.size(), &inverses[0]);

   for (tModelVertices::iterator iter = m_vertices.begin(); iter != m_vertices.end(); iter++)
   {
      int index = FloatToInt(iter->bone);
      if (index < 0)
      {
         continue;
      }

      tVec3 transformedNormal;
      inverses[index].Transform(iter->normal, &transformedNormal);
      iter->normal = transformedNormal;

      tVec3 transformedPosition;
      inverses[index].Transform(iter->pos, &transformedPosition);
      iter->pos = transformedPosition;
   }
}

///////////////////////////////////////////////////////////////////////////////
