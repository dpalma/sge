///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dread.h"
#include "ms3d.h"
#include "mesh.h"
#include "material.h"
#include "textureapi.h"
#include "readwriteapi.h"
#include "vec3.h"
#include "image.h"
#include "color.h"
#include "render.h"
#include "resmgr.h"
#include "globalobj.h"
#include "animation.h"
#include "hash.h"

#include <map>
#include <vector>
#include <algorithm>

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=31
// http://rsn.gamedev.net/tutorials/ms3danim.asp

// If this assertion fails, the struct packing was likely wrong
// when ms3d.h was compiled.
AssertOnce(sizeof(ms3d_header_t) == 14);

///////////////////////////////////////////////////////////////////////////////

struct sMs3dVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
   float bone;
};

sVertexElement g_ms3dVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2 },
   { kVDU_Normal, kVDT_Float3 },
   { kVDU_Position, kVDT_Float3 },
   { kVDU_Index, kVDT_Float1 },
};

static bool operator ==(const struct sMs3dVertex & v1,
                        const struct sMs3dVertex & v2)
{
   return v1.u == v2.u
      && v1.v == v2.v
      && v1.normal.x == v2.normal.x
      && v1.normal.y == v2.normal.y
      && v1.normal.z == v2.normal.z
      && v1.pos.x == v2.pos.x
      && v1.pos.y == v2.pos.y
      && v1.pos.z == v2.pos.z
      && v1.bone == v2.bone;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertexList
//

class cMs3dVertexList
{
public:
   cMs3dVertexList(const ms3d_vertex_t * pVertices, uint nVertices);

   uint MapVertex(uint originalIndex, tVec3 normal, float s, float t);

   const void * GetVertexData() const;
   uint GetVertexCount() const;

private:
   uint m_nOriginalVertices;
   std::vector<sMs3dVertex> m_vertices;
   std::vector<bool> m_haveVertex;
   std::map<uint, uint> m_remap;
};

///////////////////////////////////////

cMs3dVertexList::cMs3dVertexList(const ms3d_vertex_t * pVertices, uint nVertices)
 : m_nOriginalVertices(nVertices)
{
   Assert(pVertices != NULL);
   Assert(nVertices > 0);

   m_vertices.resize(nVertices);
   m_haveVertex.resize(nVertices, false);

   for (uint i = 0; i < nVertices; i++)
   {
      m_vertices[i].pos = pVertices[i].vertex;
      m_vertices[i].bone = pVertices[i].boneId;
   }
}

///////////////////////////////////////

static bool operator ==(const tVec3 & v1, const tVec3 & v2)
{
   return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
}

///////////////////////////////////////

uint cMs3dVertexList::MapVertex(uint originalIndex, tVec3 normal, float s, float t)
{
   Assert(originalIndex < m_nOriginalVertices);
   Assert(m_nOriginalVertices == m_haveVertex.size());

   // Use the complement of what is actually stored in the Milkshape file
   t = 1 - t;

   if (!m_haveVertex[originalIndex])
   {
      m_haveVertex[originalIndex] = true;
      m_vertices[originalIndex].normal = normal;
      m_vertices[originalIndex].u = s;
      m_vertices[originalIndex].v = t;
      return originalIndex;
   }
   else if ((m_vertices[originalIndex].normal == normal)
      && (m_vertices[originalIndex].u == s)
      && (m_vertices[originalIndex].v == t))
   {
      return originalIndex;
   }
   else
   {
      sMs3dVertex newVertex = m_vertices[originalIndex];
      newVertex.normal = normal;
      newVertex.u = s;
      newVertex.v = t;
      uint h = Hash(&newVertex, sizeof(newVertex));
      std::map<uint, uint>::iterator f = m_remap.find(h);
      if (f != m_remap.end())
      {
         Assert(f->second < m_vertices.size());
         return f->second;
      }
      else
      {
         // TODO: Tacking the the duplicated vertex onto the end
         // is totally not optimal with respect to vertex caches
         m_vertices.push_back(newVertex);
         uint newIndex = m_vertices.size() - 1;
         m_remap.insert(std::make_pair(h, newIndex));
         return newIndex;
      }
   }
}

///////////////////////////////////////

const void * cMs3dVertexList::GetVertexData() const
{
   return reinterpret_cast<const void *>(&m_vertices[0]);
}

///////////////////////////////////////

uint cMs3dVertexList::GetVertexCount() const
{
   return m_vertices.size();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dFileReader
//

class cMs3dFileReader
{
   friend tResult Ms3dFileRead(IRenderDevice * pRenderDevice, IReader * pReader, IMesh * * ppMesh);

public:
   cMs3dFileReader();
   ~cMs3dFileReader();

   tResult Read(IReader * pReader);

   tResult CreateMesh(IRenderDevice * pRenderDevice, IMesh * * ppMesh) const;

private:
   tResult CreateMaterials(IRenderDevice * pRenderDevice, IMesh * pMesh) const;

   static const char gm_MS3D[];

   std::vector<ms3d_vertex_t> m_vertices;
   std::vector<ms3d_triangle_t> m_triangles;
   std::vector<cMs3dGroup> m_groups;
   std::vector<ms3d_material_t> m_materials;
   std::vector<sBoneInfo> m_bones;
   std::vector<IKeyFrameInterpolator *> m_interpolators;
};

///////////////////////////////////////

const char cMs3dFileReader::gm_MS3D[] = "MS3D000000";

///////////////////////////////////////

cMs3dFileReader::cMs3dFileReader()
{
}

///////////////////////////////////////

cMs3dFileReader::~cMs3dFileReader()
{
   std::for_each(m_interpolators.begin(), m_interpolators.end(), CTInterfaceMethodRef(&IUnknown::Release));
   m_interpolators.clear();
}

///////////////////////////////////////

tResult cMs3dFileReader::Read(IReader * pReader)
{
   ms3d_header_t header;
   if (pReader->Read(&header, sizeof(header)) != S_OK ||
      memcmp(gm_MS3D, header.id, _countof(header.id)) != 0)
      return E_FAIL;

   uint16 nVertices;
   if (pReader->Read(&nVertices, sizeof(nVertices)) != S_OK
      || nVertices == 0)
      return E_FAIL;

   m_vertices.resize(nVertices);
   if (pReader->Read(&m_vertices[0], m_vertices.size() * sizeof(ms3d_vertex_t)) != S_OK)
      return E_FAIL;

   uint16 nTriangles;
   if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK
      || nTriangles == 0)
      return E_FAIL;

   m_triangles.resize(nTriangles);
   if (pReader->Read(&m_triangles[0], m_triangles.size() * sizeof(ms3d_triangle_t)) != S_OK)
      return E_FAIL;

   uint16 nGroups;
   if (pReader->Read(&nGroups, sizeof(nGroups)) != S_OK)
      return E_FAIL;

   m_groups.resize(nGroups);
   for (uint i = 0; i < nGroups; i++)
   {
      if (pReader->Read(&m_groups[i]) != S_OK)
         return E_FAIL;
   }

   uint16 nMaterials;
   if (pReader->Read(&nMaterials, sizeof(nMaterials)) != S_OK)
      return E_FAIL;

   m_materials.resize(nMaterials);
   if (pReader->Read(&m_materials[0], m_materials.size() * sizeof(ms3d_material_t)) != S_OK)
      return E_FAIL;

   if (ReadSkeleton(pReader, &m_bones, &m_interpolators) != S_OK)
      return E_FAIL;

   return S_OK;
}

///////////////////////////////////////

tResult cMs3dFileReader::CreateMesh(IRenderDevice * pRenderDevice, IMesh * * ppMesh) const
{
   cMs3dVertexList vertexList(&m_vertices[0], m_vertices.size());

   {
      // Have to construct the mapping up front so that the result of GetVertexCount() 
      // will be accurate when the vertex buffer is constructed.
      std::vector<ms3d_triangle_t>::const_iterator iter;
      for (iter = m_triangles.begin(); iter != m_triangles.end(); iter++)
      {
         for (int k = 0; k < 3; k++)
         {
            vertexList.MapVertex(
               iter->vertexIndices[k], 
               iter->vertexNormals[k], 
               iter->s[k], 
               iter->t[k]);
         }
      }
   }

   cAutoIPtr<IVertexDeclaration> pVertexDecl;
   if (pRenderDevice->CreateVertexDeclaration(g_ms3dVertexDecl, 
      _countof(g_ms3dVertexDecl), &pVertexDecl) == S_OK)
   {
      cAutoIPtr<IMesh> pMesh = MeshCreate(vertexList.GetVertexCount(), 
         kBU_Dynamic | kBU_SoftwareProcessing, pVertexDecl, pRenderDevice);
      if (!pMesh)
      {
         return E_FAIL;
      }

      sMs3dVertex * pVertexData = NULL;
      if (pMesh->LockVertexBuffer(kBL_Discard, (void * *)&pVertexData) == S_OK)
      {
         memcpy(pVertexData, vertexList.GetVertexData(), vertexList.GetVertexCount() * sizeof(sMs3dVertex));
         pMesh->UnlockVertexBuffer();
      }

      std::vector<cMs3dGroup>::const_iterator iter;
      for (iter = m_groups.begin(); iter != m_groups.end(); iter++)
      {
         cAutoIPtr<ISubMesh> pSubMesh = SubMeshCreate(iter->GetNumTriangles(), kBU_Default, pRenderDevice);
         if (!!pSubMesh)
         {
            pSubMesh->SetMaterialName(m_materials[iter->GetMaterialIndex()].name);

            uint16 * pFaces = NULL;
            if (pSubMesh->LockIndexBuffer(kBL_Discard, (void**)&pFaces) == S_OK)
            {
               for (int i = 0; i < iter->GetNumTriangles(); i++)
               {
                  const ms3d_triangle_t & tri = m_triangles[iter->GetTriangle(i)];
                  for (int k = 0; k < 3; k++)
                  {
                     pFaces[i * 3 + k] = vertexList.MapVertex(
                        tri.vertexIndices[k], 
                        tri.vertexNormals[k], 
                        tri.s[k], 
                        tri.t[k]);
                  }
               }
               pSubMesh->UnlockIndexBuffer();

               pMesh->AddSubMesh(pSubMesh);
            }
         }
      }

      if (CreateMaterials(pRenderDevice, pMesh) != S_OK)
      {
         return E_FAIL;
      }

      cAutoIPtr<ISkeleton> pSkeleton;
      if (SkeletonCreate(&m_bones[0], m_bones.size(), &pSkeleton) == S_OK)
      {
         pMesh->AttachSkeleton(pSkeleton);
      }

      cAutoIPtr<IKeyFrameAnimation> pAnimation;
      if (KeyFrameAnimationCreate(const_cast<IKeyFrameInterpolator * *>(&m_interpolators[0]), 
                                  m_interpolators.size(), 
                                  &pAnimation) == S_OK)
      {
         pSkeleton->SetAnimation(pAnimation);
      }

      if (ppMesh != NULL)
      {
         *ppMesh = pMesh;
         pMesh->AddRef();
      }

      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cMs3dFileReader::CreateMaterials(IRenderDevice * pRenderDevice, IMesh * pMesh) const
{
   std::vector<ms3d_material_t>::const_iterator iter;
   for (iter = m_materials.begin(); iter != m_materials.end(); iter++)
   {
      cAutoIPtr<IMaterial> pMaterial = MaterialCreate();
      if (pMaterial != NULL)
      {
         pMaterial->SetName(iter->name);
         pMaterial->SetAmbient(cColor(iter->ambient));
         pMaterial->SetDiffuse(cColor(iter->diffuse));
         pMaterial->SetSpecular(cColor(iter->specular));
         pMaterial->SetEmissive(cColor(iter->emissive));
         pMaterial->SetShininess(iter->shininess);

         if (iter->texture[0] != 0)
         {
            UseGlobal(TextureManager);
            cAutoIPtr<ITexture> pTexture;
            if (pTextureManager->GetTexture(iter->texture, &pTexture) == S_OK)
            {
               pMaterial->SetTexture(0, pTexture);
            }
         }

         pMesh->AddMaterial(pMaterial);
      }
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

IMesh * LoadMs3d(IRenderDevice * pRenderDevice, IReader * pReader)
{
   Assert(pRenderDevice != NULL);
   Assert(pReader != NULL);

   cMs3dFileReader ms3dReader;
   if (ms3dReader.Read(pReader) == S_OK)
   {
      cAutoIPtr<IMesh> pMesh;
      if (ms3dReader.CreateMesh(pRenderDevice, &pMesh) == S_OK)
      {
         pMesh->AddRef();
         return pMesh;
      }
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
