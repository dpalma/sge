///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"
#include "ms3dread.h"
#include "ms3d.h"
#include "skeleton.h"
#include "animation.h"

#include "renderapi.h"

#include "vec4.h"
#include "matrix4.h"
#include "resourceapi.h"
#include "readwriteapi.h"
#include "globalobj.h"
#include "filespec.h"

#include <map>
#include <windows.h> // HACK
#include <GL/gl.h>

#include "dbgalloc.h" // must be last header


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

void cModelMaterial::GlDiffuseAndTexture()
{
   glColor4fv(m_diffuse);
   uint textureId = 0;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(tResKey(m_texture.c_str(), kRC_GlTexture), (void**)&textureId) == S_OK)
   {
      glBindTexture(GL_TEXTURE_2D, textureId);
   }
}

///////////////////////////////////////
// Apply all components with glMaterial

void cModelMaterial::GlMaterialAndTexture()
{
   glMaterialfv(GL_FRONT, GL_DIFFUSE, m_diffuse);
   glMaterialfv(GL_FRONT, GL_AMBIENT, m_ambient);
   glMaterialfv(GL_FRONT, GL_SPECULAR, m_specular);
   glMaterialfv(GL_FRONT, GL_EMISSION, m_emissive);
   glMaterialfv(GL_FRONT, GL_SHININESS, &m_shininess);
   uint textureId = 0;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(tResKey(m_texture.c_str(), kRC_GlTexture), (void**)&textureId) == S_OK)
   {
      glBindTexture(GL_TEXTURE_2D, textureId);
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
 : m_indices(other.m_indices.size()),
   m_materialIndex(other.m_materialIndex)
{
   std::copy(other.m_indices.begin(), other.m_indices.end(), m_indices.begin());
}

///////////////////////////////////////

cModelMesh::cModelMesh(const std::vector<uint16> & indices, int8 materialIndex)
 : m_indices(indices.size()),
   m_materialIndex(materialIndex)
{
   std::copy(indices.begin(), indices.end(), m_indices.begin());
}

///////////////////////////////////////

cModelMesh::~cModelMesh()
{
}



///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModel
//

///////////////////////////////////////

cModel::cModel()
 : m_animationTime(0)
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

   delete pModel;

   return E_NOTIMPL;
}

///////////////////////////////////////

void cModel::Animate(double elapsedTime)
{
   //cAutoIPtr<ISkeleton> pSkeleton;
   //cAutoIPtr<IKeyFrameAnimation> pAnimation;
   //if (m_pMesh->GetSkeleton(&pSkeleton) == S_OK
   //   && pSkeleton->GetAnimation(&pAnimation) == S_OK)
   //{
   //   tTime period = pAnimation->GetPeriod();
   //   m_animationTime += elapsedTime;
   //   while (m_animationTime > period)
   //   {
   //      m_animationTime -= period;
   //   }
   //   pSkeleton->GetBoneMatrices(m_animationTime, &m_boneMatrices);
   //}
}

///////////////////////////////////////

tResult cModel::PostRead()
{
   //cAutoIPtr<ISkeleton> pSkeleton;
   //if (m_pMesh->GetSkeleton(&pSkeleton) == S_OK)
   //{
   //   m_boneMatrices.resize(pSkeleton->GetBoneCount());

   //   tMatrices inverses(pSkeleton->GetBoneCount());

   //   for (uint i = 0; i < inverses.size(); i++)
   //   {
   //      MatrixInvert(pSkeleton->GetBoneWorldTransform(i).m, inverses[i].m);
   //   }

   //   if (m_pMesh)
   //   {
   //      cAutoIPtr<IVertexBuffer> pVB;
   //      cAutoIPtr<IVertexDeclaration> pVertexDecl;

   //      // TODO: Handle sub-meshes too (not all meshes have a single shared vertex buffer)

   //      if (m_pMesh->GetVertexBuffer(&pVB) == S_OK)
   //      {
   //         sVertexElement elements[256];
   //         int nElements = _countof(elements);
   //         uint vertexSize;

   //         if (pVB->GetVertexDeclaration(&pVertexDecl) == S_OK
   //            && pVertexDecl->GetElements(elements, &nElements) == S_OK
   //            && pVertexDecl->GetVertexSize(&vertexSize) == S_OK)
   //         {
   //            uint positionOffset, normalOffset, indexOffset;

   //            for (int i = 0; i < nElements; i++)
   //            {
   //               switch (elements[i].usage)
   //               {
   //                  case kVDU_Position:
   //                  {
   //                     positionOffset = elements[i].offset;
   //                     break;
   //                  }

   //                  case kVDU_Normal:
   //                  {
   //                     normalOffset = elements[i].offset;
   //                     break;
   //                  }

   //                  case kVDU_Index:
   //                  {
   //                     indexOffset = elements[i].offset;
   //                     break;
   //                  }
   //               }
   //            }

   //            // transform all vertices by the inverse of the affecting bone's absolute matrix
   //            byte * pVertexData;
   //            if (m_pMesh->LockVertexBuffer(kBL_Default, (void**)&pVertexData) == S_OK)
   //            {
   //               for (uint i = 0; i < m_pMesh->GetVertexCount(); i++)
   //               {
   //                  byte * pVertexBase = pVertexData + (i * vertexSize);

   //                  float * pPosition = reinterpret_cast<float *>(pVertexBase + positionOffset);
   //                  float * pNormal = reinterpret_cast<float *>(pVertexBase + normalOffset);
   //                  const float * pIndex = reinterpret_cast<const float *>(pVertexBase + indexOffset);

   //                  int index = (int)*pIndex;

   //                  // TODO: No size-checking is done for position and normal members
   //                  // (i.e., float1, float2, float3, etc.)

   //                  if (index >= 0)
   //                  {
   //                     tVec4 normal(pNormal[0],pNormal[1],pNormal[2],1);
   //                     tVec4 position(pPosition[0],pPosition[1],pPosition[2],1);

   //                     tVec4 nprime;
   //                     inverses[index].Transform(normal, &nprime);
   //                     memcpy(pNormal, nprime.v, 3 * sizeof(float));

   //                     tVec4 vprime;
   //                     inverses[index].Transform(position, &vprime);
   //                     memcpy(pPosition, vprime.v, 3 * sizeof(float));
   //                  }
   //               }

   //               m_pMesh->UnlockVertexBuffer();
   //            }
   //         }
   //      }
   //   }

   //   Animate(0);
   //}

   return S_OK;
}

///////////////////////////////////////

void cModel::Render()
{
   //if (m_pMesh != NULL)
   //{
   //   if (!m_boneMatrices.empty())
   //   {
   //      pRenderDevice->SetBlendMatrices(m_boneMatrices.size(), &m_boneMatrices[0]);
   //   }
   //   m_pMesh->Render(pRenderDevice);
   //}
}

///////////////////////////////////////

tResult cModel::RegisterResourceFormat()
{
   UseGlobal(ResourceManager);
   return pResourceManager->RegisterFormat(kRT_Model, "ms3d", ModelLoadMs3d, NULL, ModelUnload);
}

///////////////////////////////////////

static const char g_ms3dId[] = "MS3D000000";
static const int g_ms3dVer = 4;

static bool ModelVertsEqual(const sModelVertex & vert1, const sModelVertex & vert2)
{
   if ((vert1.normal.x == vert2.normal.x)
      && (vert1.normal.y == vert2.normal.y)
      && (vert1.normal.z == vert2.normal.z)
      && (vert1.u == vert2.u)
      && (vert1.v == vert2.v))
   {
      return true;
   }
   return false;
}

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
      header.version != g_ms3dVer)
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

   std::vector<sModelVertex> vertices;
   vertices.resize(nVertices);

   typedef std::multimap<uint, uint> tVertexMap;
   tVertexMap vertexMap;

   std::vector<ms3d_triangle_t>::const_iterator iter;
   for (iter = tris.begin(); iter != tris.end(); iter++)
   {
      for (int k = 0; k < 3; k++)
      {
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

   DebugMsg2("Mapped vertex array has %d members (originally %d)\n", vertices.size(), ms3dVerts.size());

   //////////////////////////////
   // Read the groups

   uint16 nGroups;
   if (pReader->Read(&nGroups, sizeof(nGroups)) != S_OK)
   {
      return NULL;
   }

   cMs3dGroup groups[MAX_GROUPS];
   for (uint i = 0; i < nGroups; i++)
   {
      if (pReader->Read(&groups[i]) != S_OK)
      {
         return NULL;
      }
   }

   //////////////////////////////
   // Prepare the groups for the model

   tModelMeshes meshes(nGroups);

   for (uint i = 0; i < nGroups; i++)
   {
      const cMs3dGroup & group = groups[i];
      meshes[i] = cModelMesh(group.GetTriangles(), group.GetMaterialIndex());
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
      for (uint i = 0; i < nMaterials; i++)
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

   if (nJoints > 0)
   {
      for (uint i = 0; i < nJoints; i++)
      {
         byte flags;
         char name[32];
         char parentName[32];
         float rotation[3];
         float position[3];
         uint16 nKeyFramesRot;
         uint16 nKeyFramesTrans;

         if (pReader->Read(&flags) != S_OK
            || pReader->Read(name, sizeof(name)) != S_OK
            || pReader->Read(parentName, sizeof(parentName)) != S_OK
            || pReader->Read(rotation, sizeof(rotation)) != S_OK
            || pReader->Read(position, sizeof(position)) != S_OK
            || pReader->Read(&nKeyFramesRot) != S_OK
            || pReader->Read(&nKeyFramesTrans) != S_OK)
         {
            return NULL;
         }

         if (nKeyFramesRot != nKeyFramesTrans)
         {
            return NULL;
         }

         std::vector<ms3d_keyframe_rot_t> keyFramesRot(nKeyFramesRot);
         std::vector<ms3d_keyframe_pos_t> keyFramesTrans(nKeyFramesTrans);

         if (pReader->Read(&keyFramesRot[0], nKeyFramesRot * sizeof(ms3d_keyframe_rot_t)) != S_OK
            || pReader->Read(&keyFramesTrans[0], nKeyFramesTrans * sizeof(ms3d_keyframe_pos_t)) != S_OK)
         {
            return NULL;
         }

      }
   }

#if 0
   //////////////////////////////
   // Create the model

   cModel * pModel = NULL;
   if (cModel::Create(vertices, materials, meshes, &pModel) == S_OK)
   {
      return pModel;
   }
#endif

   return NULL;
}

///////////////////////////////////////

void cModel::ModelUnload(void * pData)
{
   cModel * pModel = reinterpret_cast<cModel*>(pData);
   delete pModel;
}

///////////////////////////////////////////////////////////////////////////////
