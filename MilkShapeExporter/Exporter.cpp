/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "Exporter.h"
#include "resource.h"

#include "modelapi.h"

#include "comtools.h"
#include "filespec.h"
#include "readwriteapi.h"
#include "readwriteutils.h"

#include "msLib.h"

#include "NvTriStrip.h"

#include <map>
#include <set>

#include "dbgalloc.h" // must be last header


/////////////////////////////////////////////////////////////////////////////

static void ParseAnimDescs(const char * pszString, std::vector<sModelAnimationDesc> * pAnimDescs)
{
   if (pszString != NULL && strlen(pszString) > 0)
   {
      cTokenizer<std::string, std::vector<std::string>, char> strTok;
      if (strTok.Tokenize(pszString, "\n") > 0)
      {
         std::vector<std::string>::iterator iter = strTok.m_tokens.begin(), end = strTok.m_tokens.end();
         for (; iter != end; ++iter)
         {
            std::string & animString = *iter;

            TrimLeadingSpace(&animString);
            TrimTrailingSpace(&animString);

            static const struct
            {
               eModelAnimationType type;
               const char * pszType;
            }
            animTypes[] =
            {
               { kMAT_Walk, "walk" },
               { kMAT_Run, "run" },
               { kMAT_Death, "death" },
               { kMAT_Attack, "attack" },
               { kMAT_Damage, "damage" },
               { kMAT_Idle, "idle" },
            };

            cTokenizer<std::string, std::vector<std::string>, char> strTok2;
            if (strTok2.Tokenize(iter->c_str()) == 3)
            {
               const std::string & animType = strTok2.m_tokens[2];

               for (int j = 0; j < _countof(animTypes); j++)
               {
                  if (animType.compare(animTypes[j].pszType) == 0)
                  {
                     sModelAnimationDesc animDesc;
                     animDesc.type = animTypes[j].type;
                     animDesc.start = atoi(strTok2.m_tokens[0].c_str());
                     animDesc.end = atoi(strTok2.m_tokens[1].c_str());
                     animDesc.fps = 0;
                     if (animDesc.start > 0 || animDesc.end > 0)
                     {
                        pAnimDescs->push_back(animDesc);
                     }
                     break;
                  }
               }
            }
         }
      }
   }
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cExportMesh
//

cExportMesh::cExportMesh(int materialIndex, int primitive,
                         std::vector<uint16>::const_iterator firstIndex,
                         std::vector<uint16>::const_iterator lastIndex)
 : m_materialIndex(materialIndex)
 , m_primitive(primitive)
 , m_indices(firstIndex, lastIndex)
{
}

tResult cReadWriteOps<cExportMesh>::Write(IWriter * pWriter, const cExportMesh & exportMesh)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(exportMesh.m_materialIndex) == S_OK
      && pWriter->Write(exportMesh.m_primitive) == S_OK
      && pWriter->Write(exportMesh.m_indices) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cExportAnimation
//

cExportAnimation::cExportAnimation(eModelAnimationType type,
                                   std::vector<tModelKeyFrameVector>::const_iterator firstKFV,
                                   std::vector<tModelKeyFrameVector>::const_iterator lastKFV)
 : m_type(type)
 , m_keyFrameVectors(firstKFV, lastKFV)
{
}

tResult cReadWriteOps<cExportAnimation>::Write(IWriter * pWriter, const cExportAnimation & exportAnim)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (pWriter->Write(static_cast<int>(exportAnim.m_type)) == S_OK
      && pWriter->Write(exportAnim.m_keyFrameVectors) == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cExporter
//

///////////////////////////////////////

cExporter::cExporter(msModel * pModel, bool bAutoDeleteModel)
 : m_pModel(pModel)
 , m_bAutoDeleteModel(bAutoDeleteModel)
{
}

///////////////////////////////////////

cExporter::~cExporter()
{
   if ((m_pModel != NULL) && m_bAutoDeleteModel)
   {
      msModel_Destroy(m_pModel);
   }

   m_pModel = NULL;
}

///////////////////////////////////////

void cExporter::PreProcess()
{
   CollectMeshes(m_pModel, &m_vertices, &m_meshes);

   CollectModelMaterials(m_pModel, &m_materials);

   CollectJoints(m_pModel, &m_tempJoints, &m_modelJoints);

   char szComment[1024];
   memset(szComment, 0, sizeof(szComment));
   msModel_GetComment(m_pModel, szComment, _countof(szComment));

   std::vector<sModelAnimationDesc> animDescs;
   ParseAnimDescs(szComment, &animDescs);

   int nTotalFrames = msModel_GetTotalFrames(m_pModel);

   std::vector<sModelAnimationDesc>::const_iterator iter = animDescs.begin(), end = animDescs.end();
   for (; iter != end; ++iter)
   {
      const sModelAnimationDesc & animDesc = *iter;

      std::vector<tModelKeyFrameVector> animKeyFrames(m_tempJoints.size());

      for (uint i = 0; i < m_tempJoints.size(); ++i)
      {
         const std::vector<msRotationKey> & rotKeys = m_tempJoints[i].GetRotationKeys();
         const std::vector<msPositionKey> & posKeys = m_tempJoints[i].GetPositionKeys();

         if (rotKeys.size() == posKeys.size())
         {
            std::vector<sModelKeyFrame> jointAnimKeyFrames;

            int iStart = -1, iEnd = -1;
            for (uint j = 0; j < rotKeys.size(); ++j)
            {
               const msRotationKey & rotKey = rotKeys[j];
               const msPositionKey & posKey = posKeys[j];
               uint rotFrame = FloatToInt(rotKey.fTime);
               uint posFrame = FloatToInt(posKey.fTime);
               Assert(rotFrame == posFrame);
               if (rotFrame == animDesc.start)
               {
                  iStart = j;
               }
               if (rotFrame >= animDesc.start)
               {
                  sModelKeyFrame keyFrame;
                  keyFrame.time = rotKey.fTime / 24; // 24 frames per second
                  keyFrame.translation = tVec3(posKey.Position);
                  keyFrame.rotation = QuatFromEulerAngles(tVec3(rotKey.Rotation));
                  jointAnimKeyFrames.push_back(keyFrame);
               }
               if (rotFrame >= animDesc.end)
               {
                  iEnd = j;
                  break;
               }
            }

            Assert(jointAnimKeyFrames.size() == (iEnd - iStart + 1));

            animKeyFrames[i].resize(jointAnimKeyFrames.size());
            std::copy(jointAnimKeyFrames.begin(), jointAnimKeyFrames.end(), animKeyFrames[i].begin());
         }
      }

      m_animSeqs.push_back(cExportAnimation(animDesc.type, animKeyFrames.begin(), animKeyFrames.end()));
   }

}

///////////////////////////////////////

tResult cExporter::ExportMesh(const tChar * pszFileName)
{
   if (pszFileName == NULL)
   {
      return E_POINTER;
   }

   cFileSpec exportFile(pszFileName);
   exportFile.SetFileExt(_T("sgem"));

   cAutoIPtr<IWriter> pWriter;
   if (FileWriterCreate(exportFile, kFileModeBinary, &pWriter) != S_OK)
   {
      return E_FAIL;
   }

   if (FAILED(ExportMesh(pWriter))
      || FAILED(pWriter->Write(m_modelJoints))
      || FAILED(pWriter->Write(m_animSeqs)))
   {
      return E_FAIL;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cExporter::ExportMesh(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   if (m_meshes.empty())
   {
      return S_FALSE;
   }

   tResult result = E_FAIL;

   do
   {
      if (pWriter->Write(cModelChunk<NoChunkData>(MODEL_FILE_ID_CHUNK)) != S_OK
         || pWriter->Write(cModelChunk<uint>(MODEL_VERSION_CHUNK, 1)) != S_OK)
      {
         break;
      }

      if (pWriter->Write(m_vertices) == S_OK
         && pWriter->Write(m_meshes) == S_OK
         && pWriter->Write(m_materials) == S_OK)
      {
         result = S_OK;
      }

   } while (0);

   return result;
}

void cExporter::CollectMeshes(msModel * pModel,
                              std::vector<sModelVertex> * pVertices,
                              std::vector<cExportMesh> * pMeshes)
{
   int nMeshes = msModel_GetMeshCount(pModel);

   typedef std::map<std::pair<uint16, uint16>, uint16> tVertexMap;
   tVertexMap vertexMap;

   for (int i = 0; i < nMeshes; ++i)
   {
      msMesh * pMesh = msModel_GetMeshAt(pModel, i);
      if (pMesh != NULL)
      {
         int iMaterial = msMesh_GetMaterialIndex(pMesh);

         word meshVertexBase = pVertices->size();
         CollectMeshVertices(pMesh, pVertices);

         std::set<uint8> meshBones;
         std::vector<sModelVertex>::iterator iter = pVertices->begin() + meshVertexBase;
         for (; iter != pVertices->end(); ++iter)
         {
            meshBones.insert((uint8)iter->bone);
         }

         if (meshBones.size() > 24)
         {
            WarnMsg1("Mesh uses %d bones\n", meshBones.size());
         }

         std::vector<tVec3> normals;
         CollectMeshNormals(pMesh, &normals);

         std::vector<uint16> mappedIndices;

         int nTris = msMesh_GetTriangleCount(pMesh);

         for (int j = 0; j < nTris; ++j)
         {
            msTriangle * pTri = msMesh_GetTriangleAt(pMesh, j);
            if (pTri != NULL)
            {
               word vertexIndices[3], normalIndices[3];
               msTriangle_GetVertexIndices(pTri, vertexIndices);
               msTriangle_GetNormalIndices(pTri, normalIndices);

               for (int k = 0; k < 3; ++k)
               {
                  vertexIndices[k] += meshVertexBase;
                  normalIndices[k] += meshVertexBase;

                  std::pair<uint16, uint16> p = std::make_pair(vertexIndices[k], normalIndices[k]);

                  tVertexMap::iterator f = vertexMap.find(p);
                  if (f != vertexMap.end())
                  {
                     mappedIndices.push_back(f->second);
                  }
                  else
                  {
                     sModelVertex newVertex = (*pVertices)[vertexIndices[k]];
                     newVertex.normal = normals[normalIndices[k]];

                     uint16 newIndex = pVertices->size();
                     pVertices->push_back(newVertex);

                     mappedIndices.push_back(newIndex);

                     vertexMap[p] = newIndex;
                  }
               }
            }
         }

         // Mapped index array must be a multiple of three
         Assert((mappedIndices.size() % 3) == 0);

         PrimitiveGroup * pPrimGroups = NULL;
         uint16 nPrimGroups = 0;
         SetStitchStrips(true);
         if (GenerateStrips(&mappedIndices[0], mappedIndices.size(), &pPrimGroups, &nPrimGroups, true))
         {
            // Should be only one strip since "stitch strips" was set to true
            Assert(nPrimGroups == 1);

            std::vector<uint16> strippedIndices(pPrimGroups->numIndices);
            std::copy(&pPrimGroups->indices[0], &pPrimGroups->indices[pPrimGroups->numIndices], strippedIndices.begin());

            //PrimitiveGroup * pPrimGroup = pPrimGroups;
            //for (int j = 0; j < nPrimGroups; ++j, ++pPrimGroup)
            //{
            //}

            pMeshes->push_back(cExportMesh(iMaterial, (int)pPrimGroups->type, strippedIndices.begin(), strippedIndices.end()));

            delete [] pPrimGroups;
         }
         else
         {
            pMeshes->push_back(cExportMesh(iMaterial, (int)pPrimGroups->type, mappedIndices.begin(), mappedIndices.end()));
         }

         msMesh_Destroy(pMesh);
      }
   }
}

void cExporter::CollectMeshVertices(msMesh * pMesh, std::vector<sModelVertex> * pVertices)
{
   int nVertices = msMesh_GetVertexCount(pMesh);

   int iStart = pVertices->size();
   int iEnd = iStart + nVertices;

   pVertices->resize(nVertices);

   for (int j = iStart; j < iEnd; j++)
   {
      msVertex * pVertex = msMesh_GetVertexAt(pMesh, j);
      if (pVertex != NULL)
      {
         sModelVertex & v = (*pVertices)[j];
         v.u = pVertex->u;
         v.v = 1 - pVertex->v;
         v.pos = tVec3(pVertex->Vertex);
         v.normal = tVec3(0,0,0);
         v.bone = pVertex->nBoneIndex;
      }
   }
}

void cExporter::CollectMeshNormals(msMesh * pMesh, std::vector<tVec3> * pNormals)
{
   int nNormals = msMesh_GetVertexNormalCount(pMesh);

   pNormals->resize(nNormals);

   for (int j = 0; j < nNormals; j++)
   {
      msVec3 normal;
      msMesh_GetVertexNormalAt(pMesh, j, normal);
      (*pNormals)[j] = tVec3(normal);
   }
}

void cExporter::CollectModelMaterials(msModel * pModel, std::vector<sModelMaterial> * pMaterials)
{
   int nMaterials = msModel_GetMaterialCount(pModel);

   pMaterials->resize(nMaterials);

   for (int i = 0; i < nMaterials; ++i)
   {
      msMaterial * pMsMaterial = msModel_GetMaterialAt(pModel, i);
      if (pMsMaterial != NULL)
      {
         char szName[MS_MAX_NAME];
         msMaterial_GetName(pMsMaterial, szName, MS_MAX_NAME);

         sModelMaterial & m = (*pMaterials)[i];

         msMaterial_GetAmbient(pMsMaterial, m.ambient);
         msMaterial_GetDiffuse(pMsMaterial, m.diffuse);
         msMaterial_GetSpecular(pMsMaterial, m.specular);
         msMaterial_GetEmissive(pMsMaterial, m.emissive);
         m.shininess = msMaterial_GetShininess(pMsMaterial);

         char szTexture[MS_MAX_PATH];
         msMaterial_GetDiffuseTexture(pMsMaterial, szTexture, _countof(szTexture));

         // Remove file extension so any image format may be used when loading this mesh into the game
         cStr texture;
         cFileSpec(szTexture).GetFileNameNoExt(&texture);
         strncpy(m.szTexture, texture.c_str(), _countof(m.szTexture));
         m.szTexture[_countof(m.szTexture) - 1] = 0;
      }
   }
}

void cExporter::CollectJoints(msModel * pModel, std::vector<cIntermediateJoint> * pTempJoints, std::vector<sModelJoint> * pModelJoints)
{
   int nBones = msModel_GetBoneCount(pModel);

   if (nBones > 0)
   {
      pTempJoints->resize(nBones);

      std::map<cStr, int> jointNameMap; // map name to index

      for (int i = 0; i < nBones; ++i)
      {
         msBone * pBone = msModel_GetBoneAt(pModel, i);

         if (pBone == NULL)
         {
            continue;
         }

         (*pTempJoints)[i] = cIntermediateJoint(pBone);

         jointNameMap.insert(std::make_pair(pBone->szName, i));
      }

      std::vector<cIntermediateJoint>::iterator iter = pTempJoints->begin();
      for (; iter != pTempJoints->end(); ++iter)
      {
         sModelJoint modelJoint;
         modelJoint.parentIndex = -1;
         modelJoint.localTranslation = tVec3(iter->GetPosition());
         modelJoint.localRotation = QuatFromEulerAngles(tVec3(iter->GetRotation()));

         if (strlen(iter->GetParentName()) > 0)
         {
            std::map<cStr, int>::iterator found = jointNameMap.find(iter->GetParentName());
            if (found != jointNameMap.end())
            {
               modelJoint.parentIndex = found->second;
            }
         }

         pModelJoints->push_back(modelJoint);
      }
   }
}

/////////////////////////////////////////////////////////////////////////////
