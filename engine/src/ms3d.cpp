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

#include <vector>

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=31
// http://rsn.gamedev.net/tutorials/ms3danim.asp

static const char g_MS3D[] = "MS3D000000";

// If this assertion fails, the struct packing was likely wrong
// when ms3d.h was compiled.
AssertOnce(sizeof(ms3d_header_t) == 14);

extern ISubMesh * SubMeshCreate(uint nFaces, uint nVertices,
                                IVertexDeclaration * pVertexDecl,
                                IRenderDevice * pRenderDevice);

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

///////////////////////////////////////////////////////////////////////////////

tResult Ms3dFileRead(IRenderDevice * pRenderDevice, IReader * pReader, IMesh * * ppMesh)
{
   ms3d_header_t header;
   if (pReader->Read(&header, sizeof(header)) != S_OK ||
      memcmp(g_MS3D, header.id, _countof(header.id)) != 0)
      return E_FAIL;

   uint16 nVertices;
   if (pReader->Read(&nVertices, sizeof(nVertices)) != S_OK
      || nVertices == 0)
      return E_FAIL;

   std::vector<ms3d_vertex_t> vertices(nVertices);
   if (pReader->Read(&vertices[0], vertices.size() * sizeof(ms3d_vertex_t)) != S_OK)
      return E_FAIL;

   uint16 nTriangles;
   if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK
      || nTriangles == 0)
      return E_FAIL;

   std::vector<ms3d_triangle_t> triangles(nTriangles);
   if (pReader->Read(&triangles[0], triangles.size() * sizeof(ms3d_triangle_t)) != S_OK)
      return E_FAIL;

   uint16 nGroups;
   if (pReader->Read(&nGroups, sizeof(nGroups)) != S_OK)
      return E_FAIL;

   uint i;

   std::vector<cMs3dGroup> groups(nGroups);
   for (i = 0; i < nGroups; i++)
   {
      if (pReader->Read(&groups[i]) != S_OK)
         return E_FAIL;
   }

   uint16 nMaterials;
   if (pReader->Read(&nMaterials, sizeof(nMaterials)) != S_OK)
      return E_FAIL;

   std::vector<ms3d_material_t> materials(nMaterials);
   if (pReader->Read(&materials[0], materials.size() * sizeof(ms3d_material_t)) != S_OK)
      return E_FAIL;

   std::vector<sMs3dVertex> vertices2(nVertices);
   for (i = 0; i < nVertices; i++)
   {
      vertices2[i].pos = tVec3(vertices[i].vertex);
   }

   // TODO: A vertex position could have a slightly different normal or 
   // texture coordinates for each triangle it's shared by. In that case,
   // the vertex should be duplicated and the indices remapped. For now,
   // keep a single copy of the vertex and ignore any discrepancies in
   // the normal or texture coordinates. Seems to work OK.
   for (i = 0; i < nTriangles; i++)
   {
      const ms3d_triangle_t & tri = triangles[i];
      for (int j = 0; j < 3; j++)
      {
         sMs3dVertex * pVertex = &vertices2[tri.vertexIndices[j]];
         pVertex->normal = tVec3(tri.vertexNormals[j]);
         pVertex->u = tri.s[j];
         pVertex->v = 1 - tri.t[j];
      }
   }

   cAutoIPtr<IMesh> pMesh = MeshCreate();
   if (!pMesh)
   {
      return E_FAIL;
   }

   std::vector<cMs3dGroup>::iterator iter;
   for (iter = groups.begin(); iter != groups.end(); iter++)
   {
      cAutoIPtr<IVertexDeclaration> pVertexDecl;
      if (pRenderDevice->CreateVertexDeclaration(g_ms3dVertexDecl, 
         _countof(g_ms3dVertexDecl), &pVertexDecl) == S_OK)
      {
         cAutoIPtr<ISubMesh> pSubMesh = SubMeshCreate(iter->GetNumTriangles(),
            vertices2.size(), pVertexDecl, pRenderDevice);
         if (!!pSubMesh)
         {
            pSubMesh->SetMaterialName(materials[iter->GetMaterialIndex()].name);

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
                     const ms3d_triangle_t & tri = triangles[iter->GetTriangle(i)];
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

   for (i = 0; i < nMaterials; i++)
   {
      const ms3d_material_t & material = materials[i];

      cAutoIPtr<ITexture> pTexture;

      if (material.texture[0] != 0)
      {
         UseGlobal(ResourceManager);

         cImage texture;
         if (ImageLoad(pResourceManager, material.texture, &texture))
         {
            pRenderDevice->CreateTexture(&texture, &pTexture);
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

   if (ppMesh != NULL)
   {
      *ppMesh = pMesh;
      pMesh->AddRef();
   }

   float animationFPS;
   float currentTime;
   int nTotalFrames;
   uint16 nJoints;
   if (pReader->Read(&animationFPS, sizeof(animationFPS)) != S_OK
      || pReader->Read(&currentTime, sizeof(currentTime)) != S_OK
      || pReader->Read(&nTotalFrames, sizeof(nTotalFrames)) != S_OK
      || pReader->Read(&nJoints, sizeof(nJoints)) != S_OK)
      return E_FAIL;

   std::vector<cMs3dJoint> joints(nJoints);
   for (i = 0; i < nJoints; i++)
   {
      if (pReader->Read(&joints[i]) != S_OK)
         break;
#ifdef _DEBUG
      joints[i].DebugPrint();
#endif
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
