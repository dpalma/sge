///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dread.h"
#include "ms3d.h"
#include "mesh.h"
#include "material.h"
#include "readwriteapi.h"
#include "vec3.h"
#include "image.h"
#include "color.h"
#include "render.h"
#include "resmgr.h"
#include "globalobj.h"
#include "animation.h"

#include <vector>
#include <algorithm>

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=31
// http://rsn.gamedev.net/tutorials/ms3danim.asp

static const char g_MS3D[] = "MS3D000000";

// If this assertion fails, the struct packing was likely wrong
// when ms3d.h was compiled.
AssertOnce(sizeof(ms3d_header_t) == 14);

///////////////////////////////////////////////////////////////////////////////

template <>
std::vector<IKeyFrameInterpolator *>::~vector()
{
   std::for_each(begin(), end(), CTInterfaceMethodRef(&IKeyFrameInterpolator::Release));
   clear();
}

///////////////////////////////////////////////////////////////////////////////

struct sMs3dVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
};

sVertexElement g_ms3dVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2 },
   { kVDU_Normal, kVDT_Float3 },
   { kVDU_Position, kVDT_Float3 }
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
      && v1.pos.z == v2.pos.z;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dVertexInfo
//
// Used to compile information on vertices stored in Milkshape3D files.
// Milkshape files store some vertex information in the polygon structure.

class cMs3dVertexInfo
{
public:
   cMs3dVertexInfo();
   cMs3dVertexInfo(const cMs3dVertexInfo & other);

   const cMs3dVertexInfo & operator =(const cMs3dVertexInfo & other);

   void SetPosition(const float pos[3]);
   const tVec3 & GetPosition() const;

   void SetBone(int bone);
   int GetBone() const;

   void AddReferringTriangle(int triIndex, int vertIndex, const float normal[3], float s, float t);

   void Digest();

private:
   uint m_index;
   tVec3 m_position;
   int m_bone;

   struct sReferringTriangle
   {
      int triIndex;
      int vertIndex;
      tVec3 normal;
      float s;
      float t;
   };

   std::vector<sReferringTriangle> m_tris;
};

///////////////////////////////////////

cMs3dVertexInfo::cMs3dVertexInfo()
 : m_bone(-1)
{
}

///////////////////////////////////////

cMs3dVertexInfo::cMs3dVertexInfo(const cMs3dVertexInfo & other)
 : m_position(other.m_position),
   m_bone(other.m_bone)
{
   m_tris.resize(other.m_tris.size());
   std::copy(other.m_tris.begin(), other.m_tris.end(), m_tris.begin());
}

///////////////////////////////////////

const cMs3dVertexInfo & cMs3dVertexInfo::operator =(const cMs3dVertexInfo & other)
{
   m_position = other.m_position;
   m_bone = other.m_bone;
   m_tris.resize(other.m_tris.size());
   std::copy(other.m_tris.begin(), other.m_tris.end(), m_tris.begin());
   return *this;
}

///////////////////////////////////////

void cMs3dVertexInfo::SetPosition(const float pos[3])
{
   Assert(pos != NULL);
   m_position = tVec3(pos);
}

///////////////////////////////////////

const tVec3 & cMs3dVertexInfo::GetPosition() const
{
   return m_position;
}

///////////////////////////////////////

void cMs3dVertexInfo::SetBone(int bone)
{
   m_bone = bone;
}

///////////////////////////////////////

int cMs3dVertexInfo::GetBone() const
{
   return m_bone;
}

///////////////////////////////////////

void cMs3dVertexInfo::AddReferringTriangle(int triIndex,
                                           int vertIndex, 
                                           const float normal[3], 
                                           float s, float t)
{
   sReferringTriangle tri;
   tri.triIndex = triIndex;
   tri.vertIndex = vertIndex;
   tri.normal = tVec3(normal);
   tri.s = s;
   tri.t = t;
   m_tris.push_back(tri);
}

///////////////////////////////////////

void cMs3dVertexInfo::Digest()
{
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

   tResult Read(IReader * pReader, IRenderDevice * pRenderDevice);

private:
   std::vector<ms3d_vertex_t> m_vertices;
   std::vector<ms3d_triangle_t> m_triangles;
   std::vector<cMs3dGroup> m_groups;
   std::vector<ms3d_material_t> m_materials;
   std::vector<sBoneInfo> m_bones;
   std::vector<IKeyFrameInterpolator *> m_interpolators;
};

///////////////////////////////////////

cMs3dFileReader::cMs3dFileReader()
{
}

///////////////////////////////////////

cMs3dFileReader::~cMs3dFileReader()
{
}

///////////////////////////////////////

tResult cMs3dFileReader::Read(IReader * pReader, IRenderDevice * pRenderDevice)
{
   ms3d_header_t header;
   if (pReader->Read(&header, sizeof(header)) != S_OK ||
      memcmp(g_MS3D, header.id, _countof(header.id)) != 0)
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

tResult Ms3dFileRead(IRenderDevice * pRenderDevice, IReader * pReader, IMesh * * ppMesh)
{
   cMs3dFileReader ms3dReader;

   if (ms3dReader.Read(pReader, pRenderDevice) != S_OK)
      return E_FAIL;

   std::vector<cMs3dVertexInfo> vertexInfo(ms3dReader.m_vertices.size());
   {
      uint index;
      std::vector<ms3d_vertex_t>::iterator iter;
      for (index = 0, iter = ms3dReader.m_vertices.begin(); iter != ms3dReader.m_vertices.end(); index++, iter++)
      {
         cMs3dVertexInfo * pV = &vertexInfo[index];
         pV->SetPosition(iter->vertex);
         pV->SetBone(iter->boneId);
      }
   }

   // TODO: A vertex position could have a slightly different normal or 
   // texture coordinates for each triangle it's shared by. In that case,
   // the vertex should be duplicated and the indices remapped. For now,
   // keep a single copy of the vertex and ignore any discrepancies in
   // the normal or texture coordinates. Seems to work OK.
   uint i;
   bool bHaveBoneAssignments = false;
   std::vector<sMs3dVertex> vertices2(ms3dReader.m_vertices.size());
   for (i = 0; i < ms3dReader.m_triangles.size(); i++)
   {
      const ms3d_triangle_t & tri = ms3dReader.m_triangles[i];
      for (int j = 0; j < 3; j++)
      {
         if (ms3dReader.m_vertices[tri.vertexIndices[j]].boneId >= 0)
         {
            bHaveBoneAssignments = true;
         }

         cMs3dVertexInfo * pV = &vertexInfo[tri.vertexIndices[j]];
         pV->AddReferringTriangle(i, j, tri.vertexNormals[j], tri.s[j], tri.t[j]);

         sMs3dVertex * pVertex = &vertices2[tri.vertexIndices[j]];
         pVertex->pos = tVec3(ms3dReader.m_vertices[tri.vertexIndices[j]].vertex);
         pVertex->normal = tVec3(tri.vertexNormals[j]);
         pVertex->u = tri.s[j];
         pVertex->v = 1 - tri.t[j];
      }
   }

   {
      std::vector<cMs3dVertexInfo>::iterator iter;
      for (iter = vertexInfo.begin(); iter != vertexInfo.end(); iter++)
      {
         iter->Digest();
      }
   }

   cAutoIPtr<IMesh> pMesh = MeshCreate();
   if (!pMesh)
   {
      return E_FAIL;
   }

   std::vector<cMs3dGroup>::iterator iter;
   for (iter = ms3dReader.m_groups.begin(); iter != ms3dReader.m_groups.end(); iter++)
   {
      cAutoIPtr<IVertexDeclaration> pVertexDecl;
      if (pRenderDevice->CreateVertexDeclaration(g_ms3dVertexDecl, 
         _countof(g_ms3dVertexDecl), &pVertexDecl) == S_OK)
      {
         cAutoIPtr<ISubMesh> pSubMesh = SubMeshCreate(iter->GetNumTriangles(),
            vertices2.size(), pVertexDecl, pRenderDevice);
         if (!!pSubMesh)
         {
            pSubMesh->SetMaterialName(ms3dReader.m_materials[iter->GetMaterialIndex()].name);

            sMs3dVertex * pVertexData = NULL;
            if (pSubMesh->LockVertexBuffer((void * *)&pVertexData) == S_OK)
            {
               // TODO: This gives every sub-mesh a copy of the entire top-level
               // vertex array. Should either be a single shared vertex array,
               // or each sub-mesh should be given an array of only its relevant 
               // vertices.
               memcpy(pVertexData, &vertices2[0], vertices2.size() * sizeof(sMs3dVertex));
               pSubMesh->UnlockVertexBuffer();

               int * pFaces = NULL;
               if (pSubMesh->LockIndexBuffer((void**)&pFaces) == S_OK)
               {
                  for (int i = 0; i < iter->GetNumTriangles(); i++)
                  {
                     const ms3d_triangle_t & tri = ms3dReader.m_triangles[iter->GetTriangle(i)];
                     pFaces[i * 3 + 0] = tri.vertexIndices[0];
                     pFaces[i * 3 + 1] = tri.vertexIndices[1];
                     pFaces[i * 3 + 2] = tri.vertexIndices[2];
                  }
                  pSubMesh->UnlockIndexBuffer();

                  pMesh->AddSubMesh(pSubMesh);
               }
            }
         }
      }
   }

   for (i = 0; i < ms3dReader.m_materials.size(); i++)
   {
      const ms3d_material_t & material = ms3dReader.m_materials[i];

      cAutoIPtr<ITexture> pTexture;

      if (material.texture[0] != 0)
      {
         UseGlobal(ResourceManager);

         cImage * pTextureImage = ImageLoad(pResourceManager, material.texture);
         if (pTextureImage != NULL)
         {
            pRenderDevice->CreateTexture(pTextureImage, &pTexture);
            delete pTextureImage;
         }
      }

      cAutoIPtr<IMaterial> pMaterial = MaterialCreate();
      if (pMaterial != NULL)
      {
         pMaterial->SetName(material.name);
         pMaterial->SetAmbient(cColor(material.ambient));
         pMaterial->SetDiffuse(cColor(material.diffuse));
         pMaterial->SetSpecular(cColor(material.specular));
         pMaterial->SetEmissive(cColor(material.emissive));
         pMaterial->SetShininess(material.shininess);
         pMaterial->SetTexture(0, pTexture);
         pMesh->AddMaterial(pMaterial);
      }
   }

   cAutoIPtr<ISkeleton> pSkeleton;
   if (SkeletonCreate(&ms3dReader.m_bones[0], ms3dReader.m_bones.size(), 
                      &ms3dReader.m_interpolators[0], ms3dReader.m_interpolators.size(), 
                      &pSkeleton) == S_OK)
   {
      pMesh->AttachSkeleton(pSkeleton);
   }

   if (ppMesh != NULL)
   {
      *ppMesh = pMesh;
      pMesh->AddRef();
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

IMesh * LoadMs3d(IRenderDevice * pRenderDevice, IReader * pReader)
{
   Assert(pReader != NULL);

   cAutoIPtr<IMesh> pMesh;
   if (Ms3dFileRead(pRenderDevice, pReader, &pMesh) == S_OK)
   {
      pMesh->AddRef();
      return pMesh;
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
