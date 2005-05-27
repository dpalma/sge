/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "msPlugInImpl.h"
#include "msPlugInExportDlg.h"

#include "model.h"

#include "readwriteapi.h"

#include "msLib.h"
#include "NvTriStrip.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPlugIn
//

///////////////////////////////////////

cPlugIn::cPlugIn()
{
}

///////////////////////////////////////

cPlugIn::~cPlugIn()
{
}

///////////////////////////////////////

int cPlugIn::GetType()
{
   return cMsPlugIn::eTypeExport;
}

///////////////////////////////////////

const char * cPlugIn::GetTitle()
{
   if (!m_title.IsEmpty() || (m_title.IsEmpty() && m_title.LoadString(IDS_TITLE)))
   {
      return m_title;
   }
   else
   {
      return "SGE Exporter";
   }
}

///////////////////////////////////////

int cPlugIn::Execute(msModel * pModel)
{
   if (pModel == NULL)
   {
      return -1;
   }

   int nMeshes = msModel_GetMeshCount(pModel);
   int nMaterials = msModel_GetMaterialCount(pModel);

   if (nMeshes == 0 && nMaterials == 0)
   {
      AtlMessageBox(GetFocus(), IDS_ERR_NOTHINGTOEXPORT);
      return -1;
   }

   cMsPlugInExportDlg dlg;
   if (dlg.DoModal() != IDOK)
   {
      return -1;
   }

   tModelMaterials materials(nMaterials);

   for (int i = 0; i < nMaterials; i++)
   {
      msMaterial * pMsMaterial = msModel_GetMaterialAt(pModel, i);
      if (pMsMaterial != NULL)
      {
         char szName[MS_MAX_NAME];
         msMaterial_GetName(pMsMaterial, szName, MS_MAX_NAME);

         msVec4 ambient;
         msMaterial_GetAmbient(pMsMaterial, ambient);

         msVec4 diffuse;
         msMaterial_GetDiffuse(pMsMaterial, diffuse);

         msVec4 specular;
         msMaterial_GetSpecular(pMsMaterial, specular);

         msVec4 emissive;
         msMaterial_GetEmissive(pMsMaterial, emissive);

         float shininess = msMaterial_GetShininess(pMsMaterial);

         char szTexture[MS_MAX_PATH];
         msMaterial_GetDiffuseTexture(pMsMaterial, szTexture, MS_MAX_PATH);

         materials[i] = cModelMaterial(diffuse, ambient, specular, emissive, shininess, szTexture);
      }
   }

   for (i = 0; i < nMeshes; i++)
   {
      msMesh * pMesh = msModel_GetMeshAt(pModel, i);
      if (pMesh != NULL)
      {
         int nVertices = msMesh_GetVertexCount(pMesh);
         int nNormals = msMesh_GetVertexNormalCount(pMesh);

         if (nVertices != nNormals)
         {
            AtlMessageBox(GetFocus(), "# vertices != # normals");
            return -1;
         }

         for (int j = 0; j < nVertices; j++)
         {
            msVec3 normal;
            msMesh_GetVertexNormalAt(pMesh, j, normal);

            msVertex * pVertex = msMesh_GetVertexAt(pMesh, j);
         }
      }
   }

   // Don't forget to destroy the model after an export
   msModel_Destroy (pModel);

   return 0;
}

///////////////////////////////////////

cMsPlugIn * CreatePlugIn()
{
   return new cPlugIn;
}

/////////////////////////////////////////////////////////////////////////////
