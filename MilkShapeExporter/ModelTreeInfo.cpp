/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ModelTreeInfo.h"
#include "resource.h"

#include "msLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelTreeInfo
//

////////////////////////////////////////

cModelTreeInfo::cModelTreeInfo(msModel * pModel)
 : m_pModel(pModel)
{
}

////////////////////////////////////////

BOOL cModelTreeInfo::DisplayModelInfo(CTreeCtrl * pTreeCtrl)
{
   ASSERT_VALID(pTreeCtrl);
   if (pTreeCtrl == NULL)
   {
      return FALSE;
   }

   if (!DisplayMaterials(pTreeCtrl)
      || !DisplayMeshes(pTreeCtrl))
   {
      return FALSE;
   }

   return TRUE;
}

////////////////////////////////////////

BOOL cModelTreeInfo::DisplayMaterials(CTreeCtrl * pTreeCtrl, HTREEITEM hParent)
{
   ASSERT_VALID(pTreeCtrl);

   int nMaterials = msModel_GetMaterialCount(m_pModel);
   if (nMaterials > 0)
   {
      CString str;
      VERIFY(str.LoadString(IDS_MATERIALS));

      HTREEITEM hMaterialsItem = pTreeCtrl->InsertItem(str, hParent);
      if (hMaterialsItem != NULL)
      {
         for (int i = 0; i < nMaterials; i++)
         {
            msMaterial * pMsMaterial = msModel_GetMaterialAt(m_pModel, i);
            if (pMsMaterial != NULL)
            {
               char szName[MS_MAX_NAME];
               msMaterial_GetName(pMsMaterial, szName, MS_MAX_NAME);

               HTREEITEM hItem = pTreeCtrl->InsertItem(szName, hMaterialsItem);
               if (hItem != NULL)
               {
                  msVec4 ambient;
                  msMaterial_GetAmbient(pMsMaterial, ambient);
                  str.Format(IDS_AMBIENT, ambient[0], ambient[1], ambient[2], ambient[3]);
                  pTreeCtrl->InsertItem(str, hItem);

                  msVec4 diffuse;
                  msMaterial_GetDiffuse(pMsMaterial, diffuse);
                  str.Format(IDS_DIFFUSE, diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
                  pTreeCtrl->InsertItem(str, hItem);

                  msVec4 specular;
                  msMaterial_GetSpecular(pMsMaterial, specular);
                  str.Format(IDS_SPECULAR, specular[0], specular[1], specular[2], specular[3]);
                  pTreeCtrl->InsertItem(str, hItem);

                  msVec4 emissive;
                  msMaterial_GetEmissive(pMsMaterial, emissive);
                  str.Format(IDS_EMISSIVE, emissive[0], emissive[1], emissive[2], emissive[3]);
                  pTreeCtrl->InsertItem(str, hItem);

                  str.Format(IDS_SHININESS, msMaterial_GetShininess(pMsMaterial));
                  pTreeCtrl->InsertItem(str, hItem);

                  char szTexture[MS_MAX_PATH];
                  msMaterial_GetDiffuseTexture(pMsMaterial, szTexture, MS_MAX_PATH);
                  str.Format(IDS_TEXTURE, szTexture);
                  pTreeCtrl->InsertItem(str, hItem);
               }
            }
         }
      }
   }

   return TRUE;
}

////////////////////////////////////////

BOOL cModelTreeInfo::DisplayMeshes(CTreeCtrl * pTreeCtrl, HTREEITEM hParent)
{
   ASSERT_VALID(pTreeCtrl);

   int nMeshes = msModel_GetMeshCount(m_pModel);
   if (nMeshes > 0)
   {
      CString str;
      VERIFY(str.LoadString(IDS_MESHES));

      HTREEITEM hMeshesItem = pTreeCtrl->InsertItem(str, hParent);
      if (hMeshesItem != NULL)
      {
         for (int i = 0; i < nMeshes; i++)
         {
            msMesh * pMsMesh = msModel_GetMeshAt(m_pModel, i);
            if (pMsMesh != NULL)
            {
               char szName[MS_MAX_NAME];
               msMesh_GetName(pMsMesh, szName, MS_MAX_NAME);

               HTREEITEM hItem = pTreeCtrl->InsertItem(szName, hMeshesItem);
               if (hItem != NULL)
               {
                  byte flags = msMesh_GetFlags(pMsMesh);
                  VERIFY(str.LoadString(IDS_FLAGS));
                  HTREEITEM hFlagsItem = pTreeCtrl->InsertItem(str, hItem);
                  if (hFlagsItem != NULL)
                  {
                     if (flags == 0)
                     {
                        VERIFY(str.LoadString(IDS_FLAGS_NONE));
                        pTreeCtrl->InsertItem(str, hFlagsItem);
                     }

                     if (flags & eSelected)
                     {
                        VERIFY(str.LoadString(IDS_FLAGS_SELECTED));
                        pTreeCtrl->InsertItem(str, hFlagsItem);
                     }

                     if (flags & eSelected2)
                     {
                        VERIFY(str.LoadString(IDS_FLAGS_SELECTED2));
                        pTreeCtrl->InsertItem(str, hFlagsItem);
                     }

                     if (flags & eHidden)
                     {
                        VERIFY(str.LoadString(IDS_FLAGS_HIDDEN));
                        pTreeCtrl->InsertItem(str, hFlagsItem);
                     }

                     if (flags & eDirty)
                     {
                        VERIFY(str.LoadString(IDS_FLAGS_DIRTY));
                        pTreeCtrl->InsertItem(str, hFlagsItem);
                     }

                     if (flags & eAveraged)
                     {
                        VERIFY(str.LoadString(IDS_FLAGS_AVERAGED));
                        pTreeCtrl->InsertItem(str, hFlagsItem);
                     }

                     if (flags & eUnused)
                     {
                        VERIFY(str.LoadString(IDS_FLAGS_UNUSED));
                        pTreeCtrl->InsertItem(str, hFlagsItem);
                     }
                  }

                  DisplayMeshVertices(pMsMesh, pTreeCtrl, hItem);
               }
            }
         }
      }
   }

   return TRUE;
}

////////////////////////////////////////

BOOL cModelTreeInfo::DisplayMeshVertices(msMesh * pMsMesh, CTreeCtrl * pTreeCtrl, HTREEITEM hParent)
{
   return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
