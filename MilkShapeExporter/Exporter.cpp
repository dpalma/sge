/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "Exporter.h"
#include "resource.h"

#include "comtools.h"
#include "filespec.h"
#include "readwriteapi.h"

#include "msLib.h"

#include "NvTriStrip.h"

#include <map>
#include <set>

#include "dbgalloc.h" // must be last header


/////////////////////////////////////////////////////////////////////////////

template <>
class cReadWriteOps<tVec3>
{
public:
   static tResult Write(IWriter * pWriter, const tVec3 & v);
};

tResult cReadWriteOps<tVec3>::Write(IWriter * pWriter, const tVec3 & v)
{
   pWriter->Write(v.x);
   pWriter->Write(v.y);
   pWriter->Write(v.z);
   return S_OK;
}


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
