///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"
#include "renderapi.h"

#include "vec4.h"
#include "matrix4.h"
#include "resourceapi.h"
#include "readwriteapi.h"
#include "globalobj.h"
#include "filespec.h"
#include "techmath.h"

#include <algorithm>
#include <cfloat>
#include <map>
#include <stack>

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

cModelMaterial::cModelMaterial(const float diffuse[4], const tChar * pszTexture)
 : m_texture(pszTexture != NULL ? pszTexture : "")
{
   memcpy(m_diffuse, diffuse, sizeof(m_diffuse));
}

///////////////////////////////////////
// All color components

cModelMaterial::cModelMaterial(const float diffuse[4], const float ambient[4],
                               const float specular[4], const float emissive[4],
                               float shininess, const tChar * pszTexture)
 : m_shininess(shininess),
   m_texture(pszTexture != NULL ? pszTexture : "")
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
// CLASS: cModelJoint
//

///////////////////////////////////////

cModelJoint::cModelJoint()
{
}

///////////////////////////////////////

cModelJoint::cModelJoint(const cModelJoint & other)
 : m_parentIndex(other.m_parentIndex)
 , m_localTransform(other.m_localTransform)
{
}

///////////////////////////////////////

cModelJoint::cModelJoint(int parentIndex, const tMatrix4 & localTransform)
 : m_parentIndex(parentIndex)
 , m_localTransform(localTransform)
{
}

///////////////////////////////////////

cModelJoint::~cModelJoint()
{
}

///////////////////////////////////////

const cModelJoint & cModelJoint::operator =(const cModelJoint & other)
{
   m_parentIndex = other.m_parentIndex;
   m_localTransform = other.m_localTransform;
   return *this;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelSkeleton
//

///////////////////////////////////////

cModelSkeleton::cModelSkeleton()
{
}

///////////////////////////////////////

cModelSkeleton::cModelSkeleton(const cModelSkeleton & other)
{
   operator =(other);
}

///////////////////////////////////////

cModelSkeleton::cModelSkeleton(const tModelJoints & joints)
 : m_joints(joints.size())
{
   if (!joints.empty())
   {
      std::copy(joints.begin(), joints.end(), m_joints.begin());
   }
}

///////////////////////////////////////

cModelSkeleton::~cModelSkeleton()
{
   std::multimap<eModelAnimationType, IModelAnimation *>::iterator iter = m_anims.begin();
   for (; iter != m_anims.end(); iter++)
   {
      iter->second->Release();
   }
}

///////////////////////////////////////

const cModelSkeleton & cModelSkeleton::operator =(const cModelSkeleton & other)
{
   m_joints.resize(other.m_joints.size());
   std::copy(other.m_joints.begin(), other.m_joints.end(), m_joints.begin());
   tAnimMap::const_iterator iter = other.m_anims.begin();
   for (; iter != other.m_anims.end(); iter++)
   {
      m_anims.insert(std::make_pair(iter->first, CTAddRef(iter->second)));
   }
   return *this;
}

///////////////////////////////////////

size_t cModelSkeleton::GetJointCount() const
{
   return m_joints.size();
}

///////////////////////////////////////

tResult cModelSkeleton::GetBindMatrices(size_t nMaxMatrices, tMatrix4 * pMatrices) const
{
   if (nMaxMatrices < m_joints.size())
   {
      return E_INVALIDARG;
   }

   if (pMatrices == NULL)
   {
      return E_POINTER;
   }

   uint i;
   int iRootJoint = -1;
   std::multimap<int, int> jointChildMap;
   tModelJoints::const_iterator iter = m_joints.begin();
   for (i = 0; iter != m_joints.end(); iter++, i++)
   {
      int iParent = iter->GetParentIndex();
      if (iParent >= 0)
      {
         jointChildMap.insert(std::make_pair(iParent, i));
      }
      else
      {
         Assert(iRootJoint == -1);
         iRootJoint = i;
      }
   }

   if (iRootJoint < 0)
   {
      ErrorMsg("Bad set of joints: no root\n");
      return E_FAIL;
   }

   tMatrices absolutes(m_joints.size(), tMatrix4::GetIdentity());

   std::stack<int> s;
   s.push(iRootJoint);
   while (!s.empty())
   {
      int iJoint = s.top();
      s.pop();

      int iParent = m_joints[iJoint].GetParentIndex();
      if (iParent == -1)
      {
         absolutes[iJoint] = m_joints[iJoint].GetLocalTransform();
      }
      else
      {
         absolutes[iParent].Multiply(m_joints[iJoint].GetLocalTransform(), &absolutes[iJoint]);
      }

      std::multimap<int, int>::iterator iter = jointChildMap.lower_bound(iJoint);
      std::multimap<int, int>::iterator end = jointChildMap.upper_bound(iJoint);
      for (; iter != end; iter++)
      {
         s.push(iter->second);
      }
   }

   for (i = 0; i < m_joints.size(); i++)
   {
      MatrixInvert(absolutes[i].m, pMatrices[i].m);
   }

   return S_OK;
}

///////////////////////////////////////

void cModelSkeleton::InterpolateMatrices(IModelAnimation * pAnim, double time, tMatrices * pMatrices) const
{
   pMatrices->resize(m_joints.size());

   tModelJoints::const_iterator iter = m_joints.begin();
   tModelJoints::const_iterator end = m_joints.end();
   for (uint i = 0; iter != end; iter++, i++)
   {
      tVec3 position;
      tQuat rotation;
      if (pAnim->Interpolate(i, time, &position, &rotation) == S_OK)
      {
         tMatrix4 mt, mr;

         rotation.ToMatrix(&mr);
         MatrixTranslate(position.x, position.y, position.z, &mt);

         tMatrix4 temp;
         mt.Multiply(mr, &temp);

         tMatrix4 mf;
         iter->GetLocalTransform().Multiply(temp, &mf);

         int iParent = iter->GetParentIndex();
         if (iParent < 0)
         {
            temp = mf;
         }
         else
         {
            (*pMatrices)[iParent].Multiply(mf, &temp);
         }

         (*pMatrices)[i] = temp;
      }
   }
}

///////////////////////////////////////

tResult cModelSkeleton::AddAnimation(eModelAnimationType type,
                                     IModelAnimation * pAnim)
{
   if (pAnim == NULL)
   {
      return E_POINTER;
   }
   m_anims.insert(std::make_pair(type, CTAddRef(pAnim)));
   return S_OK;
}

///////////////////////////////////////

tResult cModelSkeleton::GetAnimation(eModelAnimationType type,
                                     IModelAnimation * * ppAnim) const
{
   if (ppAnim == NULL)
   {
      return E_POINTER;
   }
   
   tAnimMap::const_iterator first = m_anims.lower_bound(type);
   if (first == m_anims.end())
   {
      return S_FALSE;
   }

   tAnimMap::const_iterator last = m_anims.upper_bound(type);

   if (first == last)
   {
      *ppAnim = CTAddRef(first->second);
      return S_OK;
   }
   else
   {
      tAnimMap::difference_type nAnims = 0;
      tAnimMap::const_iterator iter = first;
      for (; iter != last; iter++)
      {
         nAnims++;
      }
      uint i, iAnim = Rand() % nAnims;
      for (i = 0, iter = first; iter != last; i++, iter++)
      {
         if (i == iAnim)
         {
            break;
         }
      }
      *ppAnim = CTAddRef(iter->second);
      return S_OK;
   }

   return E_FAIL;
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
               const cModelSkeleton & skeleton)
 : m_vertices(verts.size())
 , m_materials(materials.size())
 , m_meshes(meshes.size())
 , m_skeleton(skeleton)
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
                       const cModelSkeleton & skeleton,
                       cModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   cModel * pModel = new cModel(verts, materials, meshes, skeleton);
   if (pModel == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pModel->PreApplyJoints();

   *ppModel = pModel;
   return S_OK;
}

///////////////////////////////////////

void cModel::ApplyJointMatrices(const tMatrices & matrices, tBlendedVertices * pVertices) const
{
   pVertices->resize(m_vertices.size());

   tModelVertices::const_iterator iter = m_vertices.begin();
   tModelVertices::const_iterator end = m_vertices.end();
   for (uint i = 0; iter != end; iter++, i++)
   {
      sBlendedVertex & v = (*pVertices)[i];
      v.u = iter->u;
      v.v = iter->v;
      // TODO: call them bones or joints???
      int iJoint = Round(iter->bone);
      if (iJoint < 0)
      {
         v.normal = iter->normal;
         v.pos = iter->pos;
      }
      else
      {
         matrices[iJoint].Transform(iter->normal, &v.normal);
         matrices[iJoint].Transform(iter->pos, &v.pos);
      }
   }
}

///////////////////////////////////////
// TODO: How does this work for more than one joint per vertex with blend weights?
// (Answer: I don't think you can pre-apply like this.)

void cModel::PreApplyJoints()
{
   tMatrices inverses(m_skeleton.GetJointCount());
   m_skeleton.GetBindMatrices(inverses.size(), &inverses[0]);

   for (tModelVertices::iterator iter = m_vertices.begin(); iter != m_vertices.end(); iter++)
   {
      int index = Round(iter->bone);
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
