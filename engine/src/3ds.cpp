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
#include "vec4.h"
#include "comtools.h"
#include "image.h"
#include "color.h"
#include "resmgr.h"
#include "globalobj.h"

#include <map>
#include <vector>
#include <cfloat>

#include "dbgalloc.h" // must be last header

class c3dsSubMesh;

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(3DSLoad);

#define LocalMsg(s)              DebugMsgEx(3DSLoad,(s))
#define LocalMsg1(s,a)           DebugMsgEx1(3DSLoad,(s),(a))
#define LocalMsg2(s,a,b)         DebugMsgEx2(3DSLoad,(s),(a),(b))
#define LocalMsg3(s,a,b,c)       DebugMsgEx3(3DSLoad,(s),(a),(b),(c))
#define LocalMsg4(s,a,b,c,d)     DebugMsgEx4(3DSLoad,(s),(a),(b),(c),(d))

///////////////////////////////////////////////////////////////////////////////

struct s3dsMaterial
{
   char szTexture[256];
   float ambient[3];
   float diffuse[3];
   float specular[3];
   float shininess;
   short shading;
   float masterScale;
};

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
//
// CLASS: c3dsMesh
//

class c3dsMesh : public cComObject<IMPLEMENTS(IMesh)>
{
   c3dsMesh(const c3dsMesh &); // un-implemented
   const c3dsMesh & operator=(const c3dsMesh &); // un-implemented

public:
   ////////////////////////////////////

   c3dsMesh(IRenderDevice * pRenderDevice);
   ~c3dsMesh();

   virtual void GetAABB(tVec3 * pMaxs, tVec3 * pMins) const;
   virtual void Render(IRenderDevice * pRenderDevice) const;

   void AddMaterial(const char * pszMaterial, const s3dsMaterial * p3dsMaterial);
   IMaterial * GetMaterial(const char * pszMaterial);

   void AddSubMesh(c3dsSubMesh * pSubMesh);

   static bool Load3dsMain(IReader * pReader, long stop, c3dsMesh * pMesh);

   IRenderDevice * AccessRenderDevice() { return m_pRenderDevice; }

private:
   static bool Load3dsMaterial(IReader * pReader, long stop, c3dsMesh * pMesh);
   static bool Load3dsTriangleMesh(IReader * pReader, long stop, c3dsMesh * pMesh);
   static bool Load3dsNamedObject(IReader * pReader, long stop, c3dsMesh * pMesh);
   static bool Load3dsMesh(IReader * pReader, long stop, c3dsMesh * pMesh);
   static bool Load3dsKeyFrameHeader(IReader * pReader, long stop, c3dsMesh * pMesh);
   static bool Load3dsKeyFrameData(IReader * pReader, long stop, c3dsMesh * pMesh);

   ////////////////////////////////////

   std::vector<IMaterial *> m_materials;

   typedef std::map<std::string, int> tMaterialNames;
   tMaterialNames m_materialNames;

   std::vector<c3dsSubMesh *> m_subMeshes;

   // Used for created vertex buffer and texture objects while reading from disk
   cAutoIPtr<IRenderDevice> m_pRenderDevice;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: c3dsSubMesh
//

class c3dsSubMesh
{
   c3dsSubMesh(const c3dsSubMesh &); // un-implemented
   const c3dsSubMesh & operator=(const c3dsSubMesh &); // un-implemented

public:
   c3dsSubMesh(c3dsMesh * pParent);
   ~c3dsSubMesh();

   void Render(IRenderDevice * pRenderDevice);

   bool AddVertexPositions(const float * pVertexData, int nVerts);
   bool AddVertexTexCoords(const float * pVertexData, int nVerts);
   bool AddFaces(const int * pFaceData, int nFaces);
   bool AddGroup(const char * pszMaterialName, const short * pFaceIndices, int nFaces);

   void GetAABB(tVec3 * pMaxs, tVec3 * pMins) const
   {
      Assert(pMaxs && pMins);
      *pMaxs = m_max;
      *pMins = m_min;
   }

   inline int GetVertexCount() const
   {
      return m_nVerts;
   }

private:
   c3dsMesh * GetParent() { return m_pParent; }

   bool CreateVertexBuffer(int nVerts);
   void CalcVertexNormals();

   struct sGroup
   {
      char szMaterial[100];
      uint nIndices; // 3 x # faces
      cAutoIPtr<IIndexBuffer> m_pIB;
   };

   c3dsMesh * m_pParent;
   long m_nVerts;
   cAutoIPtr<IVertexBuffer> m_pVB;
   std::vector<int> m_faceData;
   std::vector<sGroup> m_groups;
   tVec3 m_max, m_min;
};

///////////////////////////////////////

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


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: c3dsSubMesh
//

c3dsSubMesh::c3dsSubMesh(c3dsMesh * pParent)
 : m_pParent(pParent),
   m_nVerts(0),
   m_max(0,0,0),
   m_min(0,0,0)
{
}

///////////////////////////////////////

c3dsSubMesh::~c3dsSubMesh()
{
}

///////////////////////////////////////

void c3dsSubMesh::Render(IRenderDevice * pRenderDevice)
{
   std::vector<sGroup>::iterator iter;
   for (iter = m_groups.begin(); iter != m_groups.end(); iter++)
   {
      Assert(iter->m_pIB != NULL);

      IMaterial * pMaterial = m_pParent->GetMaterial(iter->szMaterial);
      if (pMaterial != NULL)
      {
         pRenderDevice->Render(kRP_Triangles, pMaterial,
            iter->nIndices, iter->m_pIB,
            0, GetVertexCount(), m_pVB);

         pMaterial->Release();
      }
   }
}

///////////////////////////////////////

bool c3dsSubMesh::AddVertexPositions(const float * pVertexData, int nVerts)
{
   if (!CreateVertexBuffer(nVerts))
   {
      return false;
   }

   s3dsVertex * pVerts = NULL;
   if (m_pVB->Lock((void * *)&pVerts) == S_OK)
   {
      tVec3 max(FLT_MIN, FLT_MIN, FLT_MIN);
      tVec3 min(FLT_MAX, FLT_MAX, FLT_MAX);

      const float * pVertexPos = pVertexData;
      for (int i = 0; i < nVerts; i++, pVertexPos += 3)
      {
         pVerts[i].pos = tVec3(pVertexPos[0], pVertexPos[1], pVertexPos[2]);

         if (max.x < pVerts[i].pos.x)
            max.x = pVerts[i].pos.x;
         if (min.x > pVerts[i].pos.x)
            min.x = pVerts[i].pos.x;

         if (max.y < pVerts[i].pos.y)
            max.y = pVerts[i].pos.y;
         if (min.y > pVerts[i].pos.y)
            min.y = pVerts[i].pos.y;

         if (max.z < pVerts[i].pos.z)
            max.z = pVerts[i].pos.z;
         if (min.z > pVerts[i].pos.z)
            min.z = pVerts[i].pos.z;
      }

      m_max = max;
      m_min = min;

      m_pVB->Unlock();

      CalcVertexNormals();

      return true;
   }

   return false;
}

///////////////////////////////////////

bool c3dsSubMesh::AddVertexTexCoords(const float * pVertexData, int nVerts)
{
   if (!CreateVertexBuffer(nVerts))
   {
      return false;
   }

   s3dsVertex * pVerts = NULL;
   if (m_pVB->Lock((void * *)&pVerts) == S_OK)
   {
      const float * pVertexPos = pVertexData;
      for (int i = 0; i < nVerts; i++, pVertexPos += 2)
      {
         pVerts[i].u = pVertexPos[0];
         pVerts[i].v = pVertexPos[1];
      }

      m_pVB->Unlock();

      return true;
   }

   return false;
}

///////////////////////////////////////

bool c3dsSubMesh::AddFaces(const int * pFaceData, int nFaces)
{
   if (m_faceData.size() > 0)
   {
      DebugMsg("Multiple calls to AddFace not supported\n");
      return false;
   }

   m_faceData.resize(3 * nFaces);

   for (int i = 0; i < nFaces; i++)
   {
      m_faceData[i * 3 + 0] = pFaceData[i * 3 + 0];
      m_faceData[i * 3 + 1] = pFaceData[i * 3 + 1];
      m_faceData[i * 3 + 2] = pFaceData[i * 3 + 2];
   }

   CalcVertexNormals();

   return true;
}

///////////////////////////////////////

bool c3dsSubMesh::AddGroup(const char * pszMaterialName, const short * pFaceIndices, int nFaces)
{
   sGroup group;
   strcpy(group.szMaterial, pszMaterialName);

   group.nIndices = 3 * nFaces;

   if (GetParent()->AccessRenderDevice()->CreateIndexBuffer(3 * nFaces, kMP_Auto, &group.m_pIB) != S_OK)
   {
      return false;
   }

   int * pFaces = NULL;
   if (group.m_pIB->Lock((void * *)&pFaces) == S_OK)
   {
      for (int i = 0; i < nFaces; i++)
      {
         pFaces[i * 3 + 0] = m_faceData[pFaceIndices[i] * 3 + 0];
         pFaces[i * 3 + 1] = m_faceData[pFaceIndices[i] * 3 + 1];
         pFaces[i * 3 + 2] = m_faceData[pFaceIndices[i] * 3 + 2];
      }

      group.m_pIB->Unlock();

      m_groups.push_back(group);

      return true;
   }

   return false;
}

///////////////////////////////////////

bool c3dsSubMesh::CreateVertexBuffer(int nVerts)
{
   if (m_nVerts > 0 && m_nVerts != nVerts)
   {
      Assert(m_pVB != NULL);
      DebugMsg("Already have vertex data and number of vertices in call doesn't match\n");
      return false;
   }

   if (!m_pVB)
   {
      cAutoIPtr<IVertexDeclaration> pVertexDecl;
      if (GetParent()->AccessRenderDevice()->CreateVertexDeclaration(g_3dsVertexDecl, _countof(g_3dsVertexDecl), &pVertexDecl) != S_OK)
      {
         return false;
      }

      if (GetParent()->AccessRenderDevice()->CreateVertexBuffer(nVerts, pVertexDecl, kMP_Auto, &m_pVB) != S_OK)
      {
         return false;
      }

      m_nVerts = nVerts;
   }

   return true;
}

///////////////////////////////////////

void c3dsSubMesh::CalcVertexNormals()
{
   if (!m_faceData.empty() && m_pVB != NULL)
   {
      const int nFaces = m_faceData.size() / 3;

      std::vector<tVec3> faceNormals;
      faceNormals.resize(nFaces);

      s3dsVertex * pVerts = NULL;
      if (m_pVB->Lock((void * *)&pVerts) == S_OK)
      {
         int i;

         for (i = 0; i < nFaces; i++)
         {
            tVec3 v1 = pVerts[m_faceData[i * 3 + 1]].pos - pVerts[m_faceData[i * 3 + 2]].pos;
            tVec3 v2 = pVerts[m_faceData[i * 3 + 2]].pos - pVerts[m_faceData[i * 3 + 0]].pos;
            faceNormals[i] = v1.Cross(v2);
         }

         for (i = 0; i < m_nVerts; i++)
         {
            tVec3 sum(0,0,0);
            int nSharedBy = 0;

            // look for faces that share this vertex
            for (int j = 0; j < nFaces; j++)
            {
               if (m_faceData[j * 3 + 0] == i
                  || m_faceData[j * 3 + 1] == i
                  || m_faceData[j * 3 + 2] == i)
               {
                  sum += faceNormals[j];
                  nSharedBy++;
               }
            }

            sum *= 1.0f / nSharedBy;
            sum.Normalize();

            pVerts[i].normal = sum;
         }

         m_pVB->Unlock();
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: c3dsMesh
//

c3dsMesh::c3dsMesh(IRenderDevice * pRenderDevice)
{
   if (pRenderDevice != NULL)
   {
      m_pRenderDevice = pRenderDevice;
      pRenderDevice->AddRef();
   }
}

///////////////////////////////////////

c3dsMesh::~c3dsMesh()
{
   do
   {
      std::vector<c3dsSubMesh *>::iterator iter;
      for (iter = m_subMeshes.begin(); iter != m_subMeshes.end(); iter++)
      {
         delete *iter;
      }
      m_subMeshes.clear();
   }
   while (0);

   do
   {
      std::vector<IMaterial *>::iterator iter;
      for (iter = m_materials.begin(); iter != m_materials.end(); iter++)
      {
         (*iter)->Release();
      }
      m_materials.clear();
   }
   while (0);
}

///////////////////////////////////////

void c3dsMesh::GetAABB(tVec3 * pMaxs, tVec3 * pMins) const
{
   tVec3 maxs(FLT_MIN, FLT_MIN, FLT_MIN);
   tVec3 mins(FLT_MAX, FLT_MAX, FLT_MAX);

   std::vector<c3dsSubMesh *>::const_iterator iter;
   for (iter = m_subMeshes.begin(); iter != m_subMeshes.end(); iter++)
   {
      tVec3 maxsSub, minsSub;
      (*iter)->GetAABB(&maxsSub, &minsSub);

      if (maxs.x < maxsSub.x)
         maxs.x = maxsSub.x;
      if (mins.x > minsSub.x)
         mins.x = minsSub.x;

      if (maxs.y < maxsSub.y)
         maxs.y = maxsSub.y;
      if (mins.y > minsSub.y)
         mins.y = minsSub.y;

      if (maxs.z < maxsSub.z)
         maxs.z = maxsSub.z;
      if (mins.z > minsSub.z)
         mins.z = minsSub.z;
   }

   if (pMaxs != NULL)
      *pMaxs = maxs;
   if (pMins != NULL)
      *pMins = mins;
}

///////////////////////////////////////

void c3dsMesh::Render(IRenderDevice * pRenderDevice) const
{
   std::vector<c3dsSubMesh *>::const_iterator iter;
   for (iter = m_subMeshes.begin(); iter != m_subMeshes.end(); iter++)
   {
      (*iter)->Render(pRenderDevice);
   }
}

///////////////////////////////////////

void c3dsMesh::AddMaterial(const char * pszMaterial, const s3dsMaterial * p3dsMaterial)
{
   Assert(pszMaterial != NULL);
   Assert(p3dsMaterial != NULL);

   // check for material with matching name already there
   if (!cAutoIPtr<IMaterial>(GetMaterial(pszMaterial)))
   {
      cAutoIPtr<ITexture> pTexture;

      if (p3dsMaterial->szTexture[0] != 0)
      {
         UseGlobal(ResourceManager);

         cImage texture;
         if (ImageLoad(pResourceManager, p3dsMaterial->szTexture, &texture))
         {
            AccessRenderDevice()->CreateTexture(&texture, &pTexture);
         }
      }

      IMaterial * pMaterial = MaterialCreate();

      if (pTexture != NULL)
         pMaterial->SetTexture(0, pTexture);

      pMaterial->SetAmbient(cColor(p3dsMaterial->ambient[0],p3dsMaterial->ambient[1],p3dsMaterial->ambient[2]));
      pMaterial->SetDiffuse(cColor(p3dsMaterial->diffuse[0],p3dsMaterial->diffuse[1],p3dsMaterial->diffuse[2]));
      pMaterial->SetSpecular(cColor(p3dsMaterial->specular[0],p3dsMaterial->specular[1],p3dsMaterial->specular[2]));
      pMaterial->SetShininess(p3dsMaterial->shininess);

      m_materials.push_back(pMaterial);
      int index = m_materials.size() - 1; // just did a push_back
      m_materialNames.insert(std::make_pair(pszMaterial, index));
   }
}

///////////////////////////////////////

IMaterial * c3dsMesh::GetMaterial(const char * pszMaterial)
{
   Assert(pszMaterial != NULL);
   tMaterialNames::const_iterator iter = m_materialNames.find(pszMaterial);
   if (iter != m_materialNames.end())
   {
      IMaterial * pM = m_materials[iter->second];
      Assert(pM != NULL);
      pM->AddRef();
      return pM;
   }
   return NULL;
}

///////////////////////////////////////

void c3dsMesh::AddSubMesh(c3dsSubMesh * pSubMesh)
{
   if (pSubMesh!= NULL)
      m_subMeshes.push_back(pSubMesh);
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

static void Load3dsMaterial(IReader * pReader, long stop, cStr * pName, s3dsMaterial * pMaterial)
{
   s3dsChunkHeader chunk;

   while (pReader->Tell() < stop &&
          pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case MAT_NAME:
         {
            cStr name;
            pReader->Read(&name, 0);
            if (pName != NULL)
               *pName = name;
            LocalMsg1("Reading material %s\n", name.c_str());
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

bool c3dsMesh::Load3dsMaterial(IReader * pReader, long stop, c3dsMesh * pMesh)
{
   cStr matName;

   s3dsMaterial mat;
   memset(&mat, 0, sizeof(mat));

   ::Load3dsMaterial(pReader, stop, &matName, &mat);

   if (!matName.empty())
   {
      pMesh->AddMaterial(matName, &mat);
      return true;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

bool c3dsMesh::Load3dsTriangleMesh(IReader * pReader, long stop, c3dsMesh * pMesh)
{
   c3dsSubMesh * pSubMesh = new c3dsSubMesh(pMesh);

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

            float * pVertexData = (float *)alloca(3 * count * sizeof(float));
            pReader->Read(pVertexData, 3 * count * sizeof(float));

            float * pVertexPos = pVertexData;
            for (int i = 0; i < count; i++, pVertexPos += 3)
            {
               // swap the y and z because in Max, z points up
               std::swap(pVertexPos[1], pVertexPos[2]);
               pVertexPos[2] = -pVertexPos[2];
            }

            if (!pSubMesh->AddVertexPositions(pVertexData, count))
            {
               return false;
            }

            break;
         }

         case TEX_VERTS:
         {
            short nTexCoords;
            pReader->Read(&nTexCoords);

            float * pVertexData = (float *)alloca(2 * nTexCoords * sizeof(float));
            pReader->Read(pVertexData, 2 * nTexCoords * sizeof(float));

            if (!pSubMesh->AddVertexTexCoords(pVertexData, nTexCoords))
            {
               return false;
            }

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

            std::vector<int> faces(nFaces * 3);

            for (int i = 0; i < nFaces; i++)
            {
               faces[i * 3 + 0] = pFaces[i].verts[0];
               faces[i * 3 + 1] = pFaces[i].verts[1];
               faces[i * 3 + 2] = pFaces[i].verts[2];
            }

            if (!pSubMesh->AddFaces(&faces[0], nFaces))
            {
               return false;
            }

            break;
         }

         case MSH_MAT_GROUP:
         {
            cStr materialName;
            pReader->Read(&materialName, 0);

            short nFaces;
            pReader->Read(&nFaces);

            std::vector<short> faces(nFaces);
            pReader->Read(&faces[0], nFaces * sizeof(short));

            pSubMesh->AddGroup(materialName, &faces[0], nFaces);

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

   pMesh->AddSubMesh(pSubMesh);

   return true;
}

///////////////////////////////////////////////////////////////////////////////

bool c3dsMesh::Load3dsNamedObject(IReader * pReader, long stop, c3dsMesh * pMesh)
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
            if (!Load3dsTriangleMesh(pReader, ChunkGetEnd(pReader, chunk), pMesh))
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

bool c3dsMesh::Load3dsMesh(IReader * pReader, long stop, c3dsMesh * pMesh)
{
   s3dsChunkHeader chunk;

   while (pReader->Tell() < stop &&
          pReader->Read(&chunk) == S_OK)
   {
      switch (chunk.id)
      {
         case NAMED_OBJECT:
         {
            if (!Load3dsNamedObject(pReader, ChunkGetEnd(pReader, chunk), pMesh))
            {
               return false;
            }
            break;
         }

         case MAT_ENTRY:
         {
            Load3dsMaterial(pReader, ChunkGetEnd(pReader, chunk), pMesh);
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

bool c3dsMesh::Load3dsKeyFrameHeader(IReader * pReader, long stop, c3dsMesh * pMesh)
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

bool c3dsMesh::Load3dsKeyFrameData(IReader * pReader, long stop, c3dsMesh * pMesh)
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

bool c3dsMesh::Load3dsMain(IReader * pReader, long stop, c3dsMesh * pMesh)
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
               bFoundMeshData = Load3dsMesh(pReader, ChunkGetEnd(pReader, chunk), pMesh);
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
   Assert(pReader != NULL);

   c3dsMesh * pMesh = new c3dsMesh(pRenderDevice);

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
         bValidated = c3dsMesh::Load3dsMain(pReader, ChunkGetEnd(pReader, chunk), pMesh);
      }
      else
      {
         LocalMsg2("Skipping chunk %x, length %d at the top level\n", chunk.id, chunk.length);
         ChunkSkip(pReader, chunk);
      }
   }

   if (!bValidated)
   {
      delete pMesh;
      return NULL;
   }

   return pMesh;
}

///////////////////////////////////////////////////////////////////////////////
