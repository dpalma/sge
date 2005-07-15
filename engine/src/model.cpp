///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"
#include "ms3dread.h"
#include "ms3d.h"

#include "vec4.h"
#include "matrix4.h"
#include "resourceapi.h"
#include "readwriteapi.h"
#include "globalobj.h"
#include "filespec.h"
#include "techmath.h"

#include <map>
#include <stack>
#include <cfloat>
#include <GL/glew.h>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

void GlSubmitModelVertices(const tModelVertices & verts)
{
   glDisableClientState(GL_EDGE_FLAG_ARRAY);
   glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   static const uint posOffset = offsetof(sModelVertex, pos);
   static const uint normalOffset = offsetof(sModelVertex, normal);
   static const uint texCoordOffset = offsetof(sModelVertex, u);

   const byte * pVertexData = reinterpret_cast<const byte *>(&verts[0]);

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(sModelVertex), pVertexData + posOffset);

   glEnableClientState(GL_NORMAL_ARRAY);
   glNormalPointer(GL_FLOAT, sizeof(sModelVertex), pVertexData + normalOffset);

   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, sizeof(sModelVertex), pVertexData + texCoordOffset);
}

void GlSubmitBlendedVertices(const tBlendedVertices & verts)
{
   glDisableClientState(GL_EDGE_FLAG_ARRAY);
   glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   static const uint posOffset = offsetof(sBlendedVertex, pos);
   static const uint normalOffset = offsetof(sBlendedVertex, normal);
   static const uint texCoordOffset = offsetof(sBlendedVertex, u);

   const byte * pVertexData = reinterpret_cast<const byte *>(&verts[0]);

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(sBlendedVertex), pVertexData + posOffset);

   glEnableClientState(GL_NORMAL_ARRAY);
   glNormalPointer(GL_FLOAT, sizeof(sBlendedVertex), pVertexData + normalOffset);

   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, sizeof(sBlendedVertex), pVertexData + texCoordOffset);
}


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
   glEnable(GL_COLOR_MATERIAL);
   glColorMaterial(GL_FRONT, GL_DIFFUSE);
   glColor4fv(m_diffuse);
   uint textureId = 0;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(tResKey(m_texture.c_str(), kRC_GlTexture), (void**)&textureId) == S_OK)
   {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, textureId);
   }
   else
   {
      glDisable(GL_TEXTURE_2D);
   }
}

///////////////////////////////////////
// Apply all components with glMaterial

void cModelMaterial::GlMaterialAndTexture() const
{
   glDisable(GL_COLOR_MATERIAL);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, m_diffuse);
   glMaterialfv(GL_FRONT, GL_AMBIENT, m_ambient);
   glMaterialfv(GL_FRONT, GL_SPECULAR, m_specular);
   glMaterialfv(GL_FRONT, GL_EMISSION, m_emissive);
   glMaterialfv(GL_FRONT, GL_SHININESS, &m_shininess);
   uint textureId = 0;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(tResKey(m_texture.c_str(), kRC_GlTexture), (void**)&textureId) == S_OK)
   {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, textureId);
   }
   else
   {
      glDisable(GL_TEXTURE_2D);
   }
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
 : m_glPrimitive(other.m_glPrimitive),
   m_indices(other.m_indices.size()),
   m_materialIndex(other.m_materialIndex)
{
   std::copy(other.m_indices.begin(), other.m_indices.end(), m_indices.begin());
}

///////////////////////////////////////

cModelMesh::cModelMesh(GLenum glPrimitive, const std::vector<uint16> & indices, int8 materialIndex)
 : m_glPrimitive(glPrimitive),
   m_indices(indices.size()),
   m_materialIndex(materialIndex)
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
   m_glPrimitive = other.m_glPrimitive;
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
 : m_parentIndex(other.m_parentIndex),
   m_localTransform(other.m_localTransform),
   m_keyFrames(other.m_keyFrames.size())
{
   std::copy(other.m_keyFrames.begin(), other.m_keyFrames.end(), m_keyFrames.begin());
}

///////////////////////////////////////

cModelJoint::cModelJoint(int parentIndex, const tMatrix4 & localTransform, const tModelKeyFrames & keyFrames)
 : m_parentIndex(parentIndex),
   m_localTransform(localTransform),
   m_keyFrames(keyFrames.size())
{
   std::copy(keyFrames.begin(), keyFrames.end(), m_keyFrames.begin());
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
   m_keyFrames.resize(other.m_keyFrames.size());
   std::copy(other.m_keyFrames.begin(), other.m_keyFrames.end(), m_keyFrames.begin());
   return *this;
}

///////////////////////////////////////

tResult cModelJoint::GetKeyFrame(uint index, sModelKeyFrame * pFrame) const
{
   if (pFrame == NULL)
   {
      return E_POINTER;
   }

   if (index >= m_keyFrames.size())
   {
      return E_INVALIDARG;
   }

   *pFrame = m_keyFrames[index];
   return S_OK;
}

///////////////////////////////////////

tResult cModelJoint::Interpolate(double time, tVec3 * pTrans, tQuat * pRot) const
{
   if (pTrans == NULL || pRot == NULL)
   {
      return E_POINTER;
   }

   tModelKeyFrames::const_iterator iter = m_keyFrames.begin();
   tModelKeyFrames::const_iterator prev = iter;
   tModelKeyFrames::const_iterator end = m_keyFrames.end();
   for (; iter != end; prev = iter, iter++)
   {
      if (iter->time >= time)
      {
         if (iter == prev)
         {
            *pRot = iter->rotation;
            *pTrans = iter->translation;
         }
         else
         {
            double u = (time - prev->time) / (iter->time - prev->time);
            *pRot = QuatSlerp(prev->rotation, iter->rotation, static_cast<float>(u));
            *pTrans = Vec3Lerp(prev->translation, iter->translation, (tVec3::value_type)u);
         }

         return S_OK;
      }
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

cModel::cModel(const tModelVertices & verts,
               const tModelMaterials & materials,
               const tModelMeshes & meshes)
 : m_vertices(verts.size()),
   m_materials(materials.size()),
   m_meshes(meshes.size())
{
   std::copy(verts.begin(), verts.end(), m_vertices.begin());
   std::copy(materials.begin(), materials.end(), m_materials.begin());
   std::copy(meshes.begin(), meshes.end(), m_meshes.begin());
}

///////////////////////////////////////

cModel::cModel(const tModelVertices & verts,
               const tModelMaterials & materials,
               const tModelMeshes & meshes,
               const tModelJoints & joints)
 : m_vertices(verts.size()),
   m_materials(materials.size()),
   m_meshes(meshes.size()),
   m_joints(joints.size())
{
   std::copy(verts.begin(), verts.end(), m_vertices.begin());
   std::copy(materials.begin(), materials.end(), m_materials.begin());
   std::copy(meshes.begin(), meshes.end(), m_meshes.begin());
   std::copy(joints.begin(), joints.end(), m_joints.begin());
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
                       const tModelJoints & joints,
                       cModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   int iRootJoint = -1;
   tModelJoints::const_iterator iter = joints.begin();
   tModelJoints::const_iterator end = joints.end();
   for (int i = 0; iter != end; iter++, i++)
   {
      if (iter->GetParentIndex() < 0)
      {
         if (iRootJoint >= 0)
         {
            ErrorMsg("No unique root joint");
            return E_FAIL;
         }
         iRootJoint = i;
      }
   }

   cModel * pModel = new cModel(verts, materials, meshes, joints);
   if (pModel == NULL)
   {
      return E_OUTOFMEMORY;
   }

   pModel->PreApplyJoints();

   *ppModel = pModel;
   return S_OK;
}

///////////////////////////////////////

double cModel::GetTotalAnimationLength() const
{
   if (m_joints.empty())
   {
      return 0;
   }

   double maxTime = DBL_MIN;

   tModelJoints::const_iterator iter = m_joints.begin();
   tModelJoints::const_iterator end = m_joints.end();
   for (; iter != end; iter++)
   {
      uint nKeyFrames = iter->GetKeyFrameCount();
      if (nKeyFrames > 0)
      {
         sModelKeyFrame keyFrame;
         if (iter->GetKeyFrame(nKeyFrames - 1, &keyFrame) == S_OK)
         {
            if (keyFrame.time > maxTime)
            {
               maxTime = keyFrame.time;
            }
         }
      }
   }

   return maxTime;
}

///////////////////////////////////////

void cModel::InterpolateJointMatrices(double time, tMatrices * pMatrices) const
{
   pMatrices->resize(m_joints.size());

   tModelJoints::const_iterator iter = m_joints.begin();
   tModelJoints::const_iterator end = m_joints.end();
   for (uint i = 0; iter != end; iter++, i++)
   {
      tVec3 position;
      tQuat rotation;
      if (iter->Interpolate(time, &position, &rotation) == S_OK)
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

tResult cModel::RegisterResourceFormat()
{
   UseGlobal(ResourceManager);
   return pResourceManager->RegisterFormat(kRT_Model, "ms3d", ModelLoadMs3d, NULL, ModelUnload);
}

///////////////////////////////////////
// TODO: How does this work for more than one joint per vertex with blend weights?
// (Answer: I don't think you can pre-apply like this.)

void cModel::PreApplyJoints()
{
   if (m_joints.empty())
   {
      return;
   }

   int iRootJoint = -1;
   std::multimap<int, int> jointChildMap;
   tModelJoints::iterator iter = m_joints.begin();
   tModelJoints::iterator end = m_joints.end();
   for (int i = 0; iter != end; iter++, i++)
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

   if (iRootJoint == -1)
   {
      ErrorMsg("Bad set of joints: no root\n");
      return;
   }

   {
      tMatrices absolutes(m_joints.size(), tMatrix4::GetIdentity());
      tMatrices inverses(m_joints.size(), tMatrix4::GetIdentity());

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

      for (uint i = 0; i < inverses.size(); i++)
      {
         MatrixInvert(absolutes[i].m, inverses[i].m);
      }

      tModelVertices::iterator iter = m_vertices.begin();
      tModelVertices::iterator end = m_vertices.end();
      for (; iter != end; iter++)
      {
         int index = Round(iter->bone);
         if (index >= 0)
         {
            tVec3 xformNormal;
            inverses[index].Transform(iter->normal, &xformNormal);
            iter->normal = xformNormal;

            tVec3 xformPos;
            inverses[index].Transform(iter->pos, &xformPos);
            iter->pos = xformPos;
         }
      }
   }
}


///////////////////////////////////////

static bool ModelVertsEqual(const sModelVertex & vert1, const sModelVertex & vert2)
{
   if ((vert1.u == vert2.u)
      && (vert1.v == vert2.v)
      && (vert1.normal.x == vert2.normal.x)
      && (vert1.normal.y == vert2.normal.y)
      && (vert1.normal.z == vert2.normal.z)
      && (vert1.pos.x == vert2.pos.x)
      && (vert1.pos.y == vert2.pos.y)
      && (vert1.pos.z == vert2.pos.z)
      && (vert1.bone == vert2.bone))
   {
      return true;
   }
   return false;
}

static void MatrixFromAngles(tVec3 angles, tMatrix4 * pMatrix)
{
   tMatrix4 rotX, rotY, rotZ, temp1, temp2;
   MatrixRotateX(Rad2Deg(angles.x), &rotX);
   MatrixRotateY(Rad2Deg(angles.y), &rotY);
   MatrixRotateZ(Rad2Deg(angles.z), &rotZ);
   temp1 = rotZ;
   temp1.Multiply(rotY, &temp2);
   temp2.Multiply(rotX, pMatrix);
}


static const char g_ms3dId[] = "MS3D000000";
static const int g_ms3dVer = 4; // ms3d files this version or later are supported

void * cModel::ModelLoadMs3d(IReader * pReader)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   //////////////////////////////
   // Read the header

   ms3d_header_t header;
   if (pReader->Read(&header, sizeof(header)) != S_OK ||
      memcmp(g_ms3dId, header.id, _countof(header.id)) != 0 ||
      header.version < g_ms3dVer)
   {
      return NULL;
   }

   //////////////////////////////
   // Read the vertices

   uint16 nVertices;
   if (pReader->Read(&nVertices, sizeof(nVertices)) != S_OK
      || nVertices == 0)
   {
      return NULL;
   }

   std::vector<ms3d_vertex_t> ms3dVerts(nVertices);
   if (pReader->Read(&ms3dVerts[0], nVertices * sizeof(ms3d_vertex_t)) != S_OK)
   {
      return NULL;
   }

   //////////////////////////////
   // Read the triangles

   uint16 nTriangles;
   if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK
      || nTriangles == 0)
   {
      return NULL;
   }

   std::vector<ms3d_triangle_t> tris(nTriangles);
   if (pReader->Read(&tris[0], nTriangles * sizeof(ms3d_triangle_t)) != S_OK)
   {
      return NULL;
   }

   //////////////////////////////
   // Re-map the vertices based on the triangles because Milkshape file
   // triangles contain some vertex info.

   // TODO: clean up this vertex mapping code !!!!!!!

   std::vector<sModelVertex> vertices;
   vertices.resize(nVertices);

   cMs3dVertexMapper vertexMapper(ms3dVerts);

   typedef std::multimap<uint, uint> tVertexMap;
   tVertexMap vertexMap;

   std::vector<ms3d_triangle_t>::const_iterator iter;
   for (iter = tris.begin(); iter != tris.end(); iter++)
   {
      for (int k = 0; k < 3; k++)
      {
         vertexMapper.MapVertex(
            iter->vertexIndices[k], 
            iter->vertexNormals[k], 
            iter->s[k], 
            iter->t[k]);

         uint index = iter->vertexIndices[k];
         if (vertexMap.find(index) == vertexMap.end())
         {
            vertices[index].u = iter->s[k];
            vertices[index].v = 1 - iter->t[k];
            vertices[index].normal = iter->vertexNormals[k];
            vertices[index].pos = ms3dVerts[index].vertex;
            vertices[index].bone = ms3dVerts[index].boneId;
            vertexMap.insert(std::make_pair(index,index));
         }
         else
         {
            sModelVertex vert = vertices[index];
            vert.u = iter->s[k];
            vert.v = 1 - iter->t[k];
            vert.normal = iter->vertexNormals[k];
            uint iVertMatch = ~0;
            tVertexMap::iterator viter = vertexMap.lower_bound(index);
            tVertexMap::iterator vend = vertexMap.upper_bound(index);
            for (; viter != vend; viter++)
            {
               uint index2 = viter->second;
               if (index2 != index)
               {
                  const sModelVertex & vert1 = vert;
                  const sModelVertex & vert2 = vertices[index2];
                  if ((vert1.normal.x == vert2.normal.x)
                     && (vert1.normal.y == vert2.normal.y)
                     && (vert1.normal.z == vert2.normal.z)
                     && (vert1.u == vert2.u)
                     && (vert1.v == vert2.v))
                  {
                     iVertMatch = index2;
                     break;
                  }
               }
            }
            if (iVertMatch == ~0)
            {
               // Not already mapped, but there may be a suitable vertex
               // elsewhere in the array. Have to look through whole map.
               tVertexMap::iterator viter = vertexMap.begin();
               tVertexMap::iterator vend = vertexMap.end();
               for (; viter != vend; viter++)
               {
                  const sModelVertex & vert1 = vert;
                  const sModelVertex & vert2 = vertices[viter->second];
                  if (ModelVertsEqual(vert1, vert2))
                  {
                     iVertMatch = viter->second;
                     break;
                  }
               }
               if (iVertMatch == ~0)
               {
                  // Not mapped and no usable vertex already in the array
                  // so create a new one.
                  vertices.push_back(vert);
                  vertexMap.insert(std::make_pair(index,vertices.size()-1));
               }
            }
         }
      }
   }

//   DebugMsg2("Mapped vertex array has %d members (originally %d)\n", vertices.size(), ms3dVerts.size());

   //////////////////////////////
   // Read the groups

   uint16 nGroups;
   if (pReader->Read(&nGroups, sizeof(nGroups)) != S_OK)
   {
      return NULL;
   }

   uint i;
   cMs3dGroup groups[MAX_GROUPS];
   for (i = 0; i < nGroups; i++)
   {
      if (pReader->Read(&groups[i]) != S_OK)
      {
         return NULL;
      }
   }

   //////////////////////////////
   // Prepare the groups for the model

   tModelMeshes meshes(nGroups);

   for (i = 0; i < nGroups; i++)
   {
      const cMs3dGroup & group = groups[i];

      std::vector<uint16> mappedIndices;
      for (uint j = 0; j < group.GetNumTriangles(); j++)
      {
         const ms3d_triangle_t & tri = tris[group.GetTriangle(j)];
         for (int k = 0; k < 3; k++)
         {
            mappedIndices.push_back(vertexMapper.MapVertex(
               tri.vertexIndices[k], tri.vertexNormals[k], 
               tri.s[k], tri.t[k]));
         }
      }

      meshes[i] = cModelMesh(GL_TRIANGLES, mappedIndices, group.GetMaterialIndex());
   }

   //////////////////////////////
   // Read the materials

   uint16 nMaterials;
   if (pReader->Read(&nMaterials, sizeof(nMaterials)) != S_OK)
   {
      return NULL;
   }

   std::vector<cModelMaterial> materials(nMaterials);

   if (nMaterials > 0)
   {
      for (i = 0; i < nMaterials; i++)
      {
         ms3d_material_t ms3dMat;
         if (pReader->Read(&ms3dMat, sizeof(ms3d_material_t)) != S_OK)
         {
            return NULL;
         }

         cStr texture;
         cFileSpec(ms3dMat.texture).GetFileNameNoExt(&texture);

         materials[i] = cModelMaterial(ms3dMat.diffuse, texture.c_str());
      }
   }

   //////////////////////////////
   // Read the animation info

   float animationFPS;
   float currentTime;
   int nTotalFrames;
   if (pReader->Read(&animationFPS, sizeof(animationFPS)) != S_OK
      || pReader->Read(&currentTime, sizeof(currentTime)) != S_OK
      || pReader->Read(&nTotalFrames, sizeof(nTotalFrames)) != S_OK)
   {
      return NULL;
   }

   //////////////////////////////
   // Read the joints

   uint16 nJoints;
   if (pReader->Read(&nJoints, sizeof(nJoints)) != S_OK)
   {
      return NULL;
   }

   std::vector<cModelJoint> joints(nJoints);

   if (nJoints > 0)
   {
      std::vector<cMs3dJoint> ms3dJoints(nJoints);

      std::map<cStr, int> jointNameMap;

      for (i = 0; i < nJoints; i++)
      {
         if (pReader->Read(&ms3dJoints[i]) != S_OK)
         {
            return NULL;
         }

         jointNameMap.insert(std::make_pair(ms3dJoints[i].GetName(), i));

      }

      std::vector<cMs3dJoint>::iterator iter = ms3dJoints.begin();
      std::vector<cMs3dJoint>::iterator end = ms3dJoints.end();
      for (i = 0; iter != end; iter++, i++)
      {
         int parentIndex = -1;

         if (strlen(iter->GetParentName()) > 0)
         {
            std::map<cStr, int>::iterator found = jointNameMap.find(iter->GetParentName());
            if (found != jointNameMap.end())
            {
               parentIndex = found->second;
            }
         }

         tMatrix4 mt, mr, local;
         MatrixTranslate(iter->GetPosition()[0], iter->GetPosition()[1], iter->GetPosition()[2], &mt);
         MatrixFromAngles(tVec3(iter->GetRotation()), &mr);
         mt.Multiply(mr, &local);

         AssertMsg(iter->GetKeyFramesRot().size() == iter->GetKeyFramesTrans().size(),
            "Should have been rejected by cMs3dJoint reader");

         tModelKeyFrames keyFrames(iter->GetKeyFramesRot().size());

         const std::vector<ms3d_keyframe_rot_t> & keyFramesRot = iter->GetKeyFramesRot();
         const std::vector<ms3d_keyframe_pos_t> & keyFramesTrans = iter->GetKeyFramesTrans();
         for (uint j = 0; j < keyFrames.size(); j++)
         {
            if (keyFramesRot[j].time != keyFramesTrans[j].time)
            {
               ErrorMsg("Time of rotation key frame not same as translation key frame\n");
               return NULL;
            }

            keyFrames[j].time = keyFramesRot[j].time;
            keyFrames[j].translation = tVec3(keyFramesTrans[j].position);
            keyFrames[j].rotation = QuatFromEulerAngles(tVec3(keyFramesRot[j].rotation));

//            int frame = Round(static_cast<float>(keyFrames[j].time) * animationFPS);
//            DebugMsg2("Key frame at %.3f is #%d\n", keyFrames[j].time, frame);
         }

         joints[i] = cModelJoint(parentIndex, local, keyFrames);
      }
   }

   //////////////////////////////
   // Create the model

   cModel * pModel = NULL;
   if (nJoints > 0)
   {
      if (cModel::Create(vertices, materials, meshes, joints, &pModel) == S_OK)
      {
         return pModel;
      }
   }
   else
   {
      if (cModel::Create(vertices, materials, meshes, &pModel) == S_OK)
      {
         return pModel;
      }
   }

   return NULL;
}

///////////////////////////////////////

void cModel::ModelUnload(void * pData)
{
   cModel * pModel = reinterpret_cast<cModel*>(pData);
   delete pModel;
}

///////////////////////////////////////////////////////////////////////////////
