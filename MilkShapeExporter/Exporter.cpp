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

template <>
class cReadWriteOps<sExportVertex>
{
public:
   static tResult Write(IWriter * pWriter, const sExportVertex & exportVertex);
};

tResult cReadWriteOps<sExportVertex>::Write(IWriter * pWriter, const sExportVertex & exportVertex)
{
   pWriter->Write(exportVertex.u);
   pWriter->Write(exportVertex.v);
   pWriter->Write(exportVertex.normal);
   pWriter->Write(exportVertex.pos);
   pWriter->Write(exportVertex.bone);
   return S_OK;
}


/////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<sExportMaterial>
{
public:
   static tResult Write(IWriter * pWriter, const sExportMaterial & exportMaterial);
};

tResult cReadWriteOps<sExportMaterial>::Write(IWriter * pWriter, const sExportMaterial & exportMaterial)
{
   pWriter->Write((void*)&exportMaterial.diffuse[0], sizeof(exportMaterial.diffuse));
   pWriter->Write((void*)&exportMaterial.ambient[0], sizeof(exportMaterial.ambient));
   pWriter->Write((void*)&exportMaterial.specular[0], sizeof(exportMaterial.specular));
   pWriter->Write((void*)&exportMaterial.emissive[0], sizeof(exportMaterial.emissive));
   pWriter->Write(exportMaterial.shininess);
   pWriter->Write(exportMaterial.szTexture);
   return S_OK;
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

tResult cExporter::ExportMesh(const tChar * pszFileName)
{
   if (pszFileName == NULL)
   {
      return E_POINTER;
   }

   cFileSpec exportFile(pszFileName);
   exportFile.SetFileExt("sgem");

   cAutoIPtr<IWriter> pWriter;
   if (FileWriterCreate(exportFile, &pWriter) != S_OK)
   {
      return E_FAIL;
   }

   std::vector<cIntermediateJoint> joints;

   if (FAILED(ExportMesh(pWriter))
      || FAILED(ExportSkeleton(pWriter, &joints))
      || FAILED(ExportAnimation(pWriter, joints)))
   {
      return E_FAIL;
   }

   return S_OK;
}

tResult cExporter::ExportMesh(IWriter * pWriter)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   int nMeshes = msModel_GetMeshCount(m_pModel);

   if (nMeshes < 1)
   {
      return E_FAIL;
   }

   sExportHeader header = { { 'M', 'e', 'G', 's' }, 1 };

   pWriter->Write(&header, sizeof(header));
   pWriter->Write(nMeshes);

   for (int i = 0; i < nMeshes; ++i)
   {
      msMesh * pMesh = msModel_GetMeshAt(m_pModel, i);
      if (pMesh != NULL)
      {
         int iMaterial = msMesh_GetMaterialIndex(pMesh);

         std::vector<sExportVertex> vertices;
         CollectMeshVertices(pMesh, &vertices);

         std::set<uint8> meshBones;
         std::vector<sExportVertex>::iterator iter = vertices.begin();
         for (; iter != vertices.end(); ++iter)
         {
            meshBones.insert((uint8)iter->bone);
         }

         if (meshBones.size() > 24)
         {
            WarnMsg1("Mesh uses %d bones\n", meshBones.size());
         }

         std::vector<tVec3> normals;
         CollectMeshNormals(pMesh, &normals);

         typedef std::map<std::pair<uint16, uint16>, uint16> tVertexMap;
         tVertexMap vertexMap;

         std::vector<sExportVertex> mappedVertices;
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
                  std::pair<uint16, uint16> p = std::make_pair(vertexIndices[k], normalIndices[k]);

                  tVertexMap::iterator f = vertexMap.find(p);
                  if (f != vertexMap.end())
                  {
                     mappedIndices.push_back(f->second);
                  }
                  else
                  {
                     sExportVertex newVertex = vertices[vertexIndices[k]];
                     newVertex.normal = normals[normalIndices[k]];

                     uint16 newIndex = mappedVertices.size();
                     mappedVertices.push_back(newVertex);

                     mappedIndices.push_back(newIndex);

                     vertexMap[p] = newIndex;
                  }
               }
            }
         }

         pWriter->Write(mappedVertices.size());
         {
            std::vector<sExportVertex>::iterator iter = mappedVertices.begin();
            for (; iter != mappedVertices.end(); ++iter)
            {
               pWriter->Write(*iter);
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

            pWriter->Write((int)pPrimGroups->type); // 0 - list, 1 - strip, 2 - fan
            pWriter->Write((int)pPrimGroups->numIndices);
            pWriter->Write(pPrimGroups->indices, pPrimGroups->numIndices * sizeof(uint16));

            //PrimitiveGroup * pPrimGroup = pPrimGroups;
            //for (int j = 0; j < nPrimGroups; ++j, ++pPrimGroup)
            //{
            //}

            delete [] pPrimGroups;
         }
         else
         {
            pWriter->Write((int)0); // 0 - list, 1 - strip, 2 - fan
            pWriter->Write((int)mappedIndices.size());
            pWriter->Write(&mappedIndices[0], mappedIndices.size() * sizeof(uint16));
         }

         msMesh_Destroy(pMesh);
      }
   }

   std::vector<sExportMaterial> materials;
   CollectModelMaterials(m_pModel, &materials);
   std::vector<sExportMaterial>::iterator iter = materials.begin();
   for (; iter != materials.end(); ++iter)
   {
      pWriter->Write(*iter);
   }

   return S_OK;
}

tResult cExporter::ExportSkeleton(IWriter * pWriter, std::vector<cIntermediateJoint> * pJoints)
{
   int nBones = msModel_GetBoneCount(m_pModel);

   if (nBones == 0)
   {
      return S_FALSE;
   }

   pJoints->resize(nBones);

   std::map<cStr, int> jointNameMap; // map name to index

   for (int i = 0; i < nBones; ++i)
   {
      msBone * pBone = msModel_GetBoneAt(m_pModel, i);

      if (pBone == NULL)
      {
         continue;
      }

      (*pJoints)[i] = cIntermediateJoint(pBone);

      jointNameMap.insert(std::make_pair(pBone->szName, i));
   }

   std::vector<cIntermediateJoint>::iterator iter = pJoints->begin();
   for (; iter != pJoints->end(); ++iter)
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

      if (pWriter->Write(parentIndex) != S_OK
         || pWriter->Write(tVec3(iter->GetPosition())) != S_OK
         || pWriter->Write(QuatFromEulerAngles(tVec3(iter->GetRotation()))) != S_OK)
      {
         return E_FAIL;
      }
   }

   return S_OK;
}

tResult cExporter::ExportAnimation(IWriter * pWriter, const std::vector<cIntermediateJoint> & joints)
{
   std::vector<sModelAnimationDesc> animDescs;

   char szComment[1024];
   memset(szComment, 0, sizeof(szComment));
   msModel_GetComment(m_pModel, szComment, _countof(szComment));

   if (strlen(szComment) > 0)
   {
      cTokenizer<cStr> strTok;
      if (strTok.Tokenize(szComment, _T("\n")) > 0)
      {
         std::vector<cStr>::iterator iter = strTok.m_tokens.begin(), end = strTok.m_tokens.end();
         for (; iter != end; ++iter)
         {
            cStr & animString = *iter;

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

            cTokenizer<cStr> strTok2;
            if (strTok2.Tokenize(iter->c_str()) == 3)
            {
               const cStr & animType = strTok2.m_tokens[2];

               for (int j = 0; j < _countof(animTypes); j++)
               {
                  if (animType.compare(animTypes[j].pszType) == 0)
                  {
                     sModelAnimationDesc animDesc;
                     animDesc.type = animTypes[j].type;
                     animDesc.start = _ttoi(strTok2.m_tokens[0].c_str());
                     animDesc.end = _ttoi(strTok2.m_tokens[1].c_str());
                     animDesc.fps = 0;
                     if (animDesc.start > 0 || animDesc.end > 0)
                     {
                        animDescs.push_back(animDesc);
                     }
                     break;
                  }
               }
            }
         }
      }
   }

   int nTotalFrames = msModel_GetTotalFrames(m_pModel);

   std::vector<sModelAnimationDesc>::const_iterator iter, end;
   for (iter = animDescs.begin(), end = animDescs.end(); iter != end; ++iter)
   {
      const sModelAnimationDesc & animDesc = *iter;
   }

   return S_OK;
}

void cExporter::CollectMeshVertices(msMesh * pMesh, std::vector<sExportVertex> * pVertices)
{
   int nVertices = msMesh_GetVertexCount(pMesh);

   pVertices->resize(nVertices);

   for (int j = 0; j < nVertices; j++)
   {
      msVertex * pVertex = msMesh_GetVertexAt(pMesh, j);
      if (pVertex != NULL)
      {
         (*pVertices)[j].u = pVertex->u;
         (*pVertices)[j].v = 1 - pVertex->v;
         (*pVertices)[j].pos = tVec3(pVertex->Vertex);
         (*pVertices)[j].normal = tVec3(0,0,0);
         (*pVertices)[j].bone = pVertex->nBoneIndex;
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

void cExporter::CollectModelMaterials(msModel * pModel, std::vector<sExportMaterial> * pMaterials)
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

         sExportMaterial & m = (*pMaterials)[i];

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

/////////////////////////////////////////////////////////////////////////////
