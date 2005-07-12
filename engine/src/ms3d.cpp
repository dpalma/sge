///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ms3dread.h"
#include "ms3d.h"
#include "skeleton.h"
#include "meshapi.h"
#include "materialapi.h"
#include "readwriteapi.h"
#include "vec3.h"
#include "imagedata.h"
#include "color.h"
#include "renderapi.h"
#include "resourceapi.h"
#include "globalobj.h"
#include "animation.h"

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

struct sMs3dBoneInfo
{
   char name[kMaxBoneName];
   char parentName[kMaxBoneName];
   float rotation[3];
   float position[3];
};

template <>
class cReadWriteOps<sMs3dBoneInfo>
{
public:
   static tResult Read(IReader * pReader, sMs3dBoneInfo * pBoneInfo);
};

tResult cReadWriteOps<sMs3dBoneInfo>::Read(IReader * pReader, sMs3dBoneInfo * pBoneInfo)
{
   Assert(pReader != NULL);
   Assert(pBoneInfo != NULL);

   byte flags; // SELECTED | DIRTY

   if (pReader->Read(&flags, sizeof(flags)) != S_OK
      || pReader->Read(pBoneInfo->name, sizeof(pBoneInfo->name)) != S_OK
      || pReader->Read(pBoneInfo->parentName, sizeof(pBoneInfo->parentName)) != S_OK
      || pReader->Read(pBoneInfo->rotation, sizeof(pBoneInfo->rotation)) != S_OK
      || pReader->Read(pBoneInfo->position, sizeof(pBoneInfo->position)) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

static tResult ReadKeyFrames(IReader * pReader, uint * pnKeyFrames, sKeyFrame * pKeyFrames)
{
   Assert(pReader != NULL);
   Assert(pnKeyFrames != NULL);
   Assert(pKeyFrames != NULL);

   tResult result = E_FAIL;

   do
   {
      uint i;
      uint16 nKeyFramesRot, nKeyFramesTrans;

      if (pReader->Read(&nKeyFramesRot, sizeof(nKeyFramesRot)) != S_OK)
         break;
      if (pReader->Read(&nKeyFramesTrans, sizeof(nKeyFramesTrans)) != S_OK)
         break;

      if (nKeyFramesRot != nKeyFramesTrans)
      {
         DebugMsg2("Have %d rotation and %d translation key frames\n", nKeyFramesRot, nKeyFramesTrans);
         break;
      }

      uint nKeyFrames = Min(*pnKeyFrames, nKeyFramesRot);

      ms3d_keyframe_rot_t rotationKeys[MAX_KEYFRAMES];
      if (pReader->Read(rotationKeys, nKeyFramesRot * sizeof(ms3d_keyframe_rot_t)) != S_OK)
         break;

      ms3d_keyframe_pos_t translationKeys[MAX_KEYFRAMES];
      if (pReader->Read(translationKeys, nKeyFramesTrans * sizeof(ms3d_keyframe_pos_t)) != S_OK)
         break;

      for (i = 0; i < nKeyFrames; i++)
      {
         pKeyFrames[i].time = translationKeys[i].time;
         pKeyFrames[i].translation = tVec3(translationKeys[i].position);
         pKeyFrames[i].rotation = QuatFromEulerAngles(tVec3(rotationKeys[i].rotation));
      }

      *pnKeyFrames = nKeyFrames;

      result = S_OK;
   }
   while (0);

   return result;
}

///////////////////////////////////////////////////////////////////////////////

static tResult ReadSkeleton(IReader * pReader, 
                            std::vector<sBoneInfo> * pBones,
                            std::vector<IKeyFrameInterpolator *> * pInterpolators)
{
   Assert(pReader != NULL);
   Assert(pBones != NULL);
   Assert(pInterpolators != NULL);

   tResult result = E_FAIL;

   do
   {
      float animationFPS;
      float currentTime;
      int nTotalFrames;
      uint16 nJoints;
      if (pReader->Read(&animationFPS, sizeof(animationFPS)) != S_OK
         || pReader->Read(&currentTime, sizeof(currentTime)) != S_OK
         || pReader->Read(&nTotalFrames, sizeof(nTotalFrames)) != S_OK
         || pReader->Read(&nJoints, sizeof(nJoints)) != S_OK)
      {
         break;
      }

      std::map<cStr, int> boneNames; // map bone names to indices

      std::vector<sMs3dBoneInfo> boneInfo(nJoints);

      pInterpolators->clear();

      uint i;
      for (i = 0; i < nJoints; i++)
      {
         if (pReader->Read(&boneInfo[i]) != S_OK)
            break;

         boneNames.insert(std::make_pair(cStr(boneInfo[i].name), i));

         sKeyFrame keyFrames[MAX_KEYFRAMES];
         uint nKeyFrames = _countof(keyFrames);
         if (ReadKeyFrames(pReader, &nKeyFrames, keyFrames) != S_OK)
         {
            break;
         }

         IKeyFrameInterpolator * pInterpolator = NULL;
         if (KeyFrameInterpolatorCreate(boneInfo[i].name, keyFrames, nKeyFrames, &pInterpolator) != S_OK)
         {
            SafeRelease(pInterpolator);
            break;
         }
         else
         {
            pInterpolators->push_back(pInterpolator);
         }
      }

      if (i < nJoints)
         break;

      pBones->resize(nJoints);

      for (i = 0; i < nJoints; i++)
      {
         strcpy((*pBones)[i].name, boneInfo[i].name);

         if (strlen(boneInfo[i].parentName) > 0)
         {
            std::map<cStr, int>::iterator n = boneNames.find(boneInfo[i].parentName);
            if (n != boneNames.end())
            {
               Assert(strcmp(boneInfo[n->second].name, boneInfo[i].parentName) == 0);
               (*pBones)[i].parentIndex = n->second;
            }
         }
         else
         {
            (*pBones)[i].parentIndex = -1;
         }

         tMatrix4 mt, mr;
         MatrixTranslate(boneInfo[i].position[0], boneInfo[i].position[1], boneInfo[i].position[2], &mt);
         MatrixFromAngles(tVec3(boneInfo[i].rotation), &mr);

         mt.Multiply(mr, &(*pBones)[i].localTransform);
      }

      result = S_OK;
   }
   while (0);

   return result;
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
   std::for_each(m_interpolators.begin(), m_interpolators.end(), CTInterfaceMethod(&IUnknown::Release));
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
   cMs3dVertexMapper vertexMapper(m_vertices);

   {
      // Have to construct the mapping up front so that the result of GetVertexCount() 
      // will be accurate when the vertex buffer is constructed.
      std::vector<ms3d_triangle_t>::const_iterator iter;
      for (iter = m_triangles.begin(); iter != m_triangles.end(); iter++)
      {
         for (int k = 0; k < 3; k++)
         {
            vertexMapper.MapVertex(
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
      cAutoIPtr<IMesh> pMesh = MeshCreate(vertexMapper.GetVertexCount(), 
         kBU_Dynamic | kBU_SoftwareProcessing, pVertexDecl, pRenderDevice);
      if (!pMesh)
      {
         return E_FAIL;
      }

      sMs3dVertex * pVertexData = NULL;
      if (pMesh->LockVertexBuffer(kBL_Discard, (void * *)&pVertexData) == S_OK)
      {
         memcpy(pVertexData, vertexMapper.GetVertexData(), vertexMapper.GetVertexCount() * sizeof(sMs3dVertex));
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
               for (uint i = 0; i < iter->GetNumTriangles(); i++)
               {
                  const ms3d_triangle_t & tri = m_triangles[iter->GetTriangle(i)];
                  for (int k = 0; k < 3; k++)
                  {
                     pFaces[i * 3 + k] = vertexMapper.MapVertex(
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
      if (!m_bones.empty())
      {
         if (SkeletonCreate(&m_bones[0], m_bones.size(), &pSkeleton) == S_OK)
         {
            pMesh->AttachSkeleton(pSkeleton);
         }
      }

      if (!m_interpolators.empty())
      {
         cAutoIPtr<IKeyFrameAnimation> pAnimation;
         if (KeyFrameAnimationCreate(const_cast<IKeyFrameInterpolator * *>(&m_interpolators[0]), 
                                    m_interpolators.size(), 
                                    &pAnimation) == S_OK)
         {
            if (!!pSkeleton)
            {
               pSkeleton->SetAnimation(pAnimation);
            }
         }
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
      cAutoIPtr<IMaterial> pMaterial;

      if (MaterialCreate(&pMaterial) == S_OK)
      {
         pMaterial->SetName(iter->name);
         pMaterial->SetAmbient(cColor(iter->ambient));
         pMaterial->SetDiffuse(cColor(iter->diffuse));
         pMaterial->SetSpecular(cColor(iter->specular));
         pMaterial->SetEmissive(cColor(iter->emissive));
         pMaterial->SetShininess(iter->shininess);
         pMaterial->SetTexture(0, iter->texture);
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

void * Ms3dLoad(IReader * pReader)
{
   if (pReader != NULL)
   {
      cMs3dFileReader * pMs3dReader = new cMs3dFileReader;
      if (pMs3dReader != NULL)
      {
         if (pMs3dReader->Read(pReader) == S_OK)
         {
            return pMs3dReader;
         }
      }
   }

   return NULL;
}

void * Ms3dPostload(void * pData, int dataLength, void * param)
{
   cMs3dFileReader * pMs3dReader = reinterpret_cast<cMs3dFileReader*>(pData);

   cAutoIPtr<IMesh> pMesh;
   if (pMs3dReader->CreateMesh(reinterpret_cast<IRenderDevice*>(param), &pMesh) == S_OK)
   {
      delete pMs3dReader;
      pMesh->AddRef();
      return pMesh;
   }

   delete pMs3dReader;
   return NULL;
}

void Ms3dUnload(void * pData)
{
   reinterpret_cast<IMesh*>(pData)->Release();
}

ENGINE_API tResult Ms3dFormatRegister()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      return pResourceManager->RegisterFormat(kRC_Mesh, "ms3d", Ms3dLoad, Ms3dPostload, Ms3dUnload);
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
