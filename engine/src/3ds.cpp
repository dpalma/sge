///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "render.h"
#include "3ds.h"
#include "mesh.h"
#include "material.h"
#include "readwriteapi.h"
#include "str.h"
#include "vec3.h"
#include "image.h"
#include "color.h"
#include "resmgr.h"
#include "globalobj.h"

#include <vector>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(3DSLoad);

#define LocalMsg(s)              DebugMsgEx(3DSLoad,(s))
#define LocalMsg1(s,a)           DebugMsgEx1(3DSLoad,(s),(a))
#define LocalMsg2(s,a,b)         DebugMsgEx2(3DSLoad,(s),(a),(b))
#define LocalMsg3(s,a,b,c)       DebugMsgEx3(3DSLoad,(s),(a),(b),(c))
#define LocalMsg4(s,a,b,c,d)     DebugMsgEx4(3DSLoad,(s),(a),(b),(c),(d))

///////////////////////////////////////////////////////////////////////////////

extern ISubMesh * SubMeshCreate(uint nFaces, uint nVertices,
                                IVertexDeclaration * pVertexDecl,
                                IRenderDevice * pRenderDevice);

struct s3dsVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
};

sVertexElement g_3dsVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2 },
   { kVDU_Normal, kVDT_Float3 },
   { kVDU_Position, kVDT_Float3 }
};

struct s3dsMaterial
{
   cStr name;
   char szTexture[256];
   float ambient[3];
   float diffuse[3];
   float specular[3];
   float shininess;
   short shading;
   float masterScale;
};

static void CalcVertexNormals(int * pFaces, int nFaces, s3dsVertex * pVerts, int nVerts)
{
   int i;

   std::vector<tVec3> faceNormals(nFaces);

   for (i = 0; i < nFaces; i++)
   {
      tVec3 v1 = pVerts[pFaces[i * 3 + 1]].pos - pVerts[pFaces[i * 3 + 2]].pos;
      tVec3 v2 = pVerts[pFaces[i * 3 + 2]].pos - pVerts[pFaces[i * 3 + 0]].pos;
      faceNormals[i] = v1.Cross(v2);
   }

   for (i = 0; i < nVerts; i++)
   {
      tVec3 sum(0,0,0);
      int nSharedBy = 0;

      // look for faces that share this vertex
      for (int j = 0; j < nFaces; j++)
      {
         if (pFaces[j * 3 + 0] == i
            || pFaces[j * 3 + 1] == i
            || pFaces[j * 3 + 2] == i)
         {
            sum += faceNormals[j];
            nSharedBy++;
         }
      }

      sum /= (float)nSharedBy;
      sum.Normalize();

      pVerts[i].normal = sum;
   }
}

static IMaterial * MaterialFrom3ds(const s3dsMaterial * p3dsMaterial,
                                   IRenderDevice * pRenderDevice)
{
   Assert(p3dsMaterial != NULL);

   cAutoIPtr<ITexture> pTexture;

   if (p3dsMaterial->szTexture[0] != 0)
   {
      UseGlobal(ResourceManager);

      cImage * pTextureImage = ImageLoad(pResourceManager, p3dsMaterial->szTexture);
      if (pTextureImage != NULL)
      {
         pRenderDevice->CreateTexture(pTextureImage, &pTexture);
         delete pTextureImage;
      }
   }

   IMaterial * pMaterial = MaterialCreate();

   if (pTexture != NULL)
      pMaterial->SetTexture(0, pTexture);

   pMaterial->SetName(p3dsMaterial->name);
   pMaterial->SetAmbient(cColor(p3dsMaterial->ambient[0],p3dsMaterial->ambient[1],p3dsMaterial->ambient[2]));
   pMaterial->SetDiffuse(cColor(p3dsMaterial->diffuse[0],p3dsMaterial->diffuse[1],p3dsMaterial->diffuse[2]));
   pMaterial->SetSpecular(cColor(p3dsMaterial->specular[0],p3dsMaterial->specular[1],p3dsMaterial->specular[2]));
   pMaterial->SetShininess(p3dsMaterial->shininess);

   return pMaterial;
}

///////////////////////////////////////////////////////////////////////////////

static long ChunkGetEnd(IReader * pReader, const s3dsChunkHeader & chunk)
{
   Assert(pReader != NULL);
   return pReader->Tell() + chunk.length - kSizeof3dsChunkHeader;
}

static void ChunkSkip(IReader * pReader, const s3dsChunkHeader & chunk)
{
   pReader->Seek(chunk.length - kSizeof3dsChunkHeader, kSO_Cur);
}

AssertOnce(sizeof(s3dsChunkHeader) == kSizeof3dsChunkHeader);

template <>
tResult cReadWriteOps<s3dsChunkHeader>::Read(IReader * pReader, s3dsChunkHeader * pChunk)
{
   return pReader->Read(pChunk, sizeof(*pChunk));
}


///////////////////////////////////////////////////////////////////////////////

static bool Load3dsColor(IReader * pReader, float * pColor)
{
   s3dsChunkHeader chunk;
   if (pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case COLOR_F:
         {
            pReader->Read(pColor, 3 * sizeof(float));
            return true;
         }

         case COLOR_24:
         {
            byte rgb[3];
            pReader->Read(rgb, 3 * sizeof(byte));
            static const float kOneOver255 = 1.0f / 255.0f;
            pColor[0] = (float)rgb[0] * kOneOver255;
            pColor[1] = (float)rgb[1] * kOneOver255;
            pColor[2] = (float)rgb[2] * kOneOver255;
            return true;
         }
      }
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

static bool Load3dsPercentage(IReader * pReader, float * pPercentage)
{
   s3dsChunkHeader chunk;
   if (pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case FLOAT_PERCENTAGE:
         {
            pReader->Read(pPercentage);
            return true;
         }

         case INT_PERCENTAGE:
         {
            short percentage;
            pReader->Read(&percentage);
            *pPercentage = (float)percentage * 0.001f;
            return true;
         }
      }
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

static void Load3dsTexMap(IReader * pReader, long stop, s3dsMaterial * pMaterial)
{
   s3dsChunkHeader chunk;

   while (pReader->Tell() < stop &&
          pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case MAT_MAPNAME:
         {
            cStr texture;
            pReader->Read(&texture, 0);
            strcpy(pMaterial->szTexture, texture.c_str());
            break;
         }

         default:
         {
            LocalMsg2("Skipping chunk %x, length %d in tex map\n", chunk.id, chunk.length);
            ChunkSkip(pReader, chunk);
            break;
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

static void Load3dsMaterial(IReader * pReader, long stop, s3dsMaterial * pMaterial)
{
   s3dsChunkHeader chunk;

   while (pReader->Tell() < stop &&
          pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case MAT_NAME:
         {
            pReader->Read(&pMaterial->name, 0);
            LocalMsg1("Reading material %s\n", pMaterial->name.c_str());
            break;
         }

         case MAT_SHININESS:
         {
            Verify(Load3dsPercentage(pReader, &pMaterial->shininess));
            break;
         }

         case MAT_AMBIENT:
         {
            Verify(Load3dsColor(pReader, pMaterial->ambient));
            break;
         }

         case MAT_DIFFUSE:
         {
            Verify(Load3dsColor(pReader, pMaterial->diffuse));
            break;
         }

         case MAT_SPECULAR:
         {
            Verify(Load3dsColor(pReader, pMaterial->specular));
            break;
         }

         case MAT_SHADING:
         {
            pReader->Read(&pMaterial->shading);
            break;
         }

         case MAT_TEXMAP:
         {
            Load3dsTexMap(pReader, ChunkGetEnd(pReader, chunk), pMaterial);
            break;
         }

         case MASTER_SCALE:
         {
            pReader->Read(&pMaterial->masterScale);
            break;
         }

         default:
         {
            LocalMsg2("Skipping chunk %x, length %d in material\n", chunk.id, chunk.length);
            ChunkSkip(pReader, chunk);
            break;
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

bool Load3dsMaterial(IReader * pReader, long stop, IRenderDevice * pRenderDevice, IMesh * pMesh)
{
   s3dsMaterial mat;
   memset(&mat, 0, sizeof(mat));

   ::Load3dsMaterial(pReader, stop, &mat);

   cAutoIPtr<IMaterial> pMaterial(MaterialFrom3ds(&mat, pRenderDevice));

   return (pMesh->AddMaterial(pMaterial) == S_OK);
}

///////////////////////////////////////////////////////////////////////////////

bool Load3dsTriangleMesh(IReader * pReader, long stop, IRenderDevice * pRenderDevice, IMesh * pMesh)
{
   bool bResult = false;

   bool bHavePoints = false, bHaveTexCoords = false;
   std::vector<s3dsVertex> vertices;

   std::vector<int> faces;

   s3dsChunkHeader chunk;

   while (pReader->Tell() < stop &&
          pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case POINT_ARRAY:
         {
            short count;
            pReader->Read(&count);

            Assert(!bHavePoints && vertices.empty());
            vertices.resize(count);

            float * pVertexData = (float *)alloca(3 * count * sizeof(float));
            pReader->Read(pVertexData, 3 * count * sizeof(float));

            float * pVertexPos = pVertexData;
            for (int i = 0; i < count; i++, pVertexPos += 3)
            {
               // swap the y and z because in Max, z points up
               std::swap(pVertexPos[1], pVertexPos[2]);
               pVertexPos[2] = -pVertexPos[2];

               vertices[i].pos = tVec3(pVertexPos);
            }

            bHavePoints = true;

            break;
         }

         case TEX_VERTS:
         {
            short nTexCoords;
            pReader->Read(&nTexCoords);

            Assert(bHavePoints && vertices.size() == nTexCoords);

            float * pTexCoordData = (float *)alloca(2 * nTexCoords * sizeof(float));
            pReader->Read(pTexCoordData, 2 * nTexCoords * sizeof(float));

            float * pData = pTexCoordData;
            for (int i = 0; i < nTexCoords; i++, pData += 2)
            {
               vertices[i].u = pData[0];
               vertices[i].v = pData[1];
            }

            bHaveTexCoords = true;

            break;
         }

         case MESH_MATRIX:
         {
            // @TODO: figure out how to use this
            float matrix[4][3];     
            pReader->Read(matrix, sizeof(matrix));
            break;
         }

         case FACE_ARRAY:
         {
            struct s3dsFace
            {
               short verts[3];
               short flags;
            };

            short nFaces;
            pReader->Read(&nFaces);

            s3dsFace * pFaces = (s3dsFace *)alloca(nFaces * sizeof(s3dsFace));
            pReader->Read(pFaces, nFaces * sizeof(s3dsFace));

            AssertMsg(faces.empty(), "Expecting faces array to be empty");
            faces.resize(nFaces * 3);

            for (int i = 0; i < nFaces; i++)
            {
               faces[i * 3 + 0] = pFaces[i].verts[0];
               faces[i * 3 + 1] = pFaces[i].verts[1];
               faces[i * 3 + 2] = pFaces[i].verts[2];
            }

            if (bHavePoints)
            {
               Assert(!vertices.empty());
               CalcVertexNormals(&faces[0], nFaces, &vertices[0], vertices.size());
            }

            break;
         }

         case MSH_MAT_GROUP:
         {
            cStr materialName;
            pReader->Read(&materialName, 0);

            Assert(!faces.empty() && !vertices.empty() && bHavePoints && bHaveTexCoords);

            short nGroupFaces;
            pReader->Read(&nGroupFaces);

            std::vector<short> groupFaces(nGroupFaces);
            pReader->Read(&groupFaces[0], nGroupFaces * sizeof(short));

            cAutoIPtr<IVertexDeclaration> pVertexDecl;
            if (pRenderDevice->CreateVertexDeclaration(g_3dsVertexDecl,
                                                       _countof(g_3dsVertexDecl),
                                                       &pVertexDecl) == S_OK)
            {
               cAutoIPtr<ISubMesh> pSubMesh = SubMeshCreate(faces.size() / 3,
                  vertices.size(), pVertexDecl, pRenderDevice);

               pSubMesh->SetMaterialName(materialName);

               s3dsVertex * pVertexData = NULL;
               if (pSubMesh->LockVertexBuffer((void * *)&pVertexData) == S_OK)
               {
                  memcpy(pVertexData, &vertices[0], vertices.size() * sizeof(s3dsVertex));
                  pSubMesh->UnlockVertexBuffer();

                  int * pFaces = NULL;
                  if (pSubMesh->LockIndexBuffer((void**)&pFaces) == S_OK)
                  {
                     for (int i = 0; i < nGroupFaces; i++)
                     {
                        pFaces[i * 3 + 0] = faces[groupFaces[i] * 3 + 0];
                        pFaces[i * 3 + 1] = faces[groupFaces[i] * 3 + 1];
                        pFaces[i * 3 + 2] = faces[groupFaces[i] * 3 + 2];
                     }
                     pSubMesh->UnlockIndexBuffer();

                     pMesh->AddSubMesh(pSubMesh);
                     bResult = true;
                  }
               }
            }

            break;
         }

         default:
         {
            LocalMsg3("Skipping chunk %x, length %d at file position %d in triangle mesh\n",
               chunk.id, chunk.length, pReader->Tell());
            ChunkSkip(pReader, chunk);
            break;
         }
      }
   }

   return bResult;
}

///////////////////////////////////////////////////////////////////////////////

bool Load3dsNamedObject(IReader * pReader, long stop, IRenderDevice * pRenderDevice, IMesh * pMesh)
{
   cStr name;
   pReader->Read(&name, 0);

   LocalMsg1("Reading named object %s\n", name.c_str());

   s3dsChunkHeader chunk;

   while (pReader->Tell() < stop &&
          pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case N_TRI_OBJECT:
         {
            if (!Load3dsTriangleMesh(pReader, ChunkGetEnd(pReader, chunk), pRenderDevice, pMesh))
            {
               return false;
            }
            break;
         }

         default:
         {
            LocalMsg2("Skipping chunk %x, length %d in named object\n", chunk.id, chunk.length);
            ChunkSkip(pReader, chunk);
            break;
         }
      }
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Load3dsMesh(IReader * pReader, long stop, IRenderDevice * pRenderDevice, IMesh * pMesh)
{
   s3dsChunkHeader chunk;

   while (pReader->Tell() < stop &&
          pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case NAMED_OBJECT:
         {
            if (!Load3dsNamedObject(pReader, ChunkGetEnd(pReader, chunk), pRenderDevice, pMesh))
            {
               return false;
            }
            break;
         }

         case MAT_ENTRY:
         {
            Load3dsMaterial(pReader, ChunkGetEnd(pReader, chunk), pRenderDevice, pMesh);
            break;
         }

         default:
         {
            LocalMsg2("Skipping chunk %x, length %d in mesh\n", chunk.id, chunk.length);
            ChunkSkip(pReader, chunk);
            break;
         }
      }
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Load3dsKeyFrameHeader(IReader * pReader, long stop, IMesh * pMesh)
{
   short revision;
   pReader->Read(&revision);

   cStr fileName;
   pReader->Read(&fileName, 0);

   short animLen;
   pReader->Read(&animLen);

   LocalMsg3("Keyframe Header: revision = %d, file = %s, length = %d\n",
      revision, fileName.c_str(), animLen);

   return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Load3dsKeyFrameData(IReader * pReader, long stop, IMesh * pMesh)
{
   s3dsChunkHeader chunk;

   while (pReader->Tell() < stop &&
          pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case KFHDR:
         {
            Load3dsKeyFrameHeader(pReader, ChunkGetEnd(pReader, chunk), pMesh);
            break;
         }

         default:
         {
            LocalMsg2("Skipping chunk %x, length %d in keyframe data\n", chunk.id, chunk.length);
            ChunkSkip(pReader, chunk);
            break;
         }
      }
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Load3dsMain(IReader * pReader, long stop, IRenderDevice * pRenderDevice, IMesh * pMesh)
{
   s3dsChunkHeader chunk;
   bool bFoundMeshData = false;

   while (pReader->Tell() < stop &&
          pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case MDATA:
         {
            if (bFoundMeshData)
            {
               LocalMsg("Skipping additional mesh data\n");
               ChunkSkip(pReader, chunk);
            }
            else
            {
               bFoundMeshData = Load3dsMesh(pReader, ChunkGetEnd(pReader, chunk), pRenderDevice, pMesh);
            }
            break;
         }

         case KFDATA:
         {
            Load3dsKeyFrameData(pReader, ChunkGetEnd(pReader, chunk), pMesh);
            break;
         }

         default:
         {
            LocalMsg2("Skipping chunk %x, length %d in main chunk\n", chunk.id, chunk.length);
            ChunkSkip(pReader, chunk);
            break;
         }
      }
   }

   return bFoundMeshData;
}

///////////////////////////////////////////////////////////////////////////////

IMesh * Load3ds(IRenderDevice * pRenderDevice, IReader * pReader)
{
   Assert(pRenderDevice != NULL);
   Assert(pReader != NULL);

   IMesh * pMesh = MeshCreate();

   bool bValidated = false;

   long originalPos = pReader->Tell();
   pReader->Seek(0, kSO_End);
   long totalSize = pReader->Tell();
   pReader->Seek(originalPos, kSO_Set);

   s3dsChunkHeader chunk;
   while (pReader->Read(&chunk) == S_OK)
   {
      if (chunk.id == M3DMAGIC)
      {
         bValidated = Load3dsMain(pReader, ChunkGetEnd(pReader, chunk), pRenderDevice, pMesh);
      }
      else
      {
         LocalMsg2("Skipping chunk %x, length %d at the top level\n", chunk.id, chunk.length);
         ChunkSkip(pReader, chunk);
      }
   }

   if (!bValidated)
   {
      SafeRelease(pMesh);
      return NULL;
   }

   return pMesh;
}

///////////////////////////////////////////////////////////////////////////////
