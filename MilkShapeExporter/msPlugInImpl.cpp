/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "msPlugInImpl.h"
#include "resource.h"
#include "msPlugInExportDlg.h"
#include "ModelTreeInfo.h"

#include "meshapi.h"
#include "material.h"
#include "render.h"
#include "color.h"
#include "comtools.h"

#include "msLib.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPlugIn
//

///////////////////////////////////////

cPlugIn::cPlugIn()
{
   VERIFY(m_title.LoadString(AFX_IDS_APP_TITLE));
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
   return m_title;
}

///////////////////////////////////////

int cPlugIn::Execute(msModel * pModel)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   if (pModel == NULL)
   {
      return -1;
   }

   int nMeshes = msModel_GetMeshCount(pModel);

   int nMaterials = msModel_GetMaterialCount(pModel);

   if (nMeshes == 0 && nMaterials == 0)
   {
      AfxMessageBox(IDS_ERR_NOTHINGTOEXPORT);
      return -1;
   }

   cModelTreeInfo modelTreeInfo(pModel);

   cMsPlugInExportDlg dlg(&modelTreeInfo);
   if (dlg.DoModal() != IDOK)
   {
      return -1;
   }

   cAutoIPtr<IMesh> pMesh = MeshCreate();

   for (int i = 0; i < nMaterials; i++)
   {
      msMaterial * pMsMaterial = msModel_GetMaterialAt(pModel, i);
      if (pMsMaterial != NULL)
      {
         cAutoIPtr<IMaterial> pMaterial = MaterialCreate();

         char szName[MS_MAX_NAME];
         msMaterial_GetName(pMsMaterial, szName, MS_MAX_NAME);
         pMaterial->SetName(szName);

         msVec4 ambient;
         msMaterial_GetAmbient(pMsMaterial, ambient);
         pMaterial->SetAmbient(cColor(ambient));

         msVec4 diffuse;
         msMaterial_GetDiffuse(pMsMaterial, diffuse);
         pMaterial->SetDiffuse(cColor(diffuse));

         msVec4 specular;
         msMaterial_GetSpecular(pMsMaterial, specular);
         pMaterial->SetSpecular(cColor(specular));

         msVec4 emissive;
         msMaterial_GetEmissive(pMsMaterial, emissive);
         pMaterial->SetEmissive(cColor(emissive));

         pMaterial->SetShininess(msMaterial_GetShininess(pMsMaterial));

         char szTexture[MS_MAX_PATH];
         msMaterial_GetDiffuseTexture(pMsMaterial, szTexture, MS_MAX_PATH);
         // TODO: pMaterial->SetTexture(0, szTexture);

         pMesh->AddMaterial(pMaterial);
      }
   }

#if 0
   cAutoIPtr<IRenderDevice> pRenderDevice;
   if (FAILED(RenderDeviceCreate(&pRenderDevice)))
   {
      AfxMessageBox(IDS_ERR_RENDERDEVICEFAILCREATE);
      return -1;
   }
#endif

   for (i = 0; i < nMeshes; i++)
   {
      msMesh * pMesh = msModel_GetMeshAt(pModel, i);
      if (pMesh != NULL)
      {
         int nVertices = msMesh_GetVertexCount(pMesh);
         int nNormals = msMesh_GetVertexNormalCount(pMesh);

         if (nVertices != nNormals)
         {
            AfxMessageBox("# vertices != # normals");
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
