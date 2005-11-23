/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "msPlugInExportDlg.h"

#include "msLib.h"
#include "NvTriStrip.h"

#include <atldlgs.h>

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMsPlugInExportDlg
//

////////////////////////////////////////

cMsPlugInExportDlg::cMsPlugInExportDlg(msModel * pMsModel)
 : m_bExportAnimations(false)
 , m_exportFileName(_T(""))
 , m_pMsModel(pMsModel)
{
}

////////////////////////////////////////

cMsPlugInExportDlg::~cMsPlugInExportDlg()
{
}


/////////////////////////////////////////////////////////////////////////////
// cMsPlugInExportDlg message handlers

static bool ChooseExportFileName(CString * pFileName)
{
   CString defaultExt;
   Verify(defaultExt.LoadString(IDS_EXPORT_DEFAULT_EXTENSION));

   CString filter;
   Verify(filter.LoadString(IDS_EXPORT_FILTER));

   CFileDialog dlg(FALSE, defaultExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);
   if (dlg.DoModal() == IDOK)
   {
      *pFileName = dlg.m_szFileName;
      return true;
   }

   return false;
}


LRESULT cMsPlugInExportDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
   DlgResize_Init();

   // Update the dialog controls from the member variables
   // after populating drop-lists, etc.
   DoDataExchange(FALSE);

   CenterWindow(GetParent());

   if (msModel_GetMeshCount(m_pMsModel) <= 0)
   {
      static const UINT disable[] = { IDOK, IDC_BROWSE, IDC_FILENAME, IDC_EXPORTANIMATIONS, IDC_ANIMATIONS };
      for (int i = 0; i < _countof(disable); i++)
      {
         CWindow ctrl(GetDlgItem(disable[i]));
         if (ctrl.IsWindow())
            ctrl.EnableWindow(FALSE);
      }
   }

   int nFrames = msModel_GetTotalFrames(m_pMsModel);
   if (nFrames > 0)
   {
      Verify(CheckDlgButton(IDC_EXPORTANIMATIONS, BST_CHECKED));

      CString temp;
      temp.Format("%d", nFrames);
      SetDlgItemText(IDC_TOTAL_FRAMES, temp);

      CRect rect;
      m_animations.GetClientRect(&rect);

      int smallWidth = rect.Width() / 5;

      LVCOLUMN columns[] =
      {
         { LVCF_TEXT|LVCF_WIDTH, 0, rect.Width() - (3 * smallWidth), _T("Name"), 0, 0 },
         { LVCF_TEXT|LVCF_WIDTH, 0, smallWidth, _T("Start"), 0, 1 },
         { LVCF_TEXT|LVCF_WIDTH, 0, smallWidth, _T("End"), 0, 2 },
         { LVCF_TEXT|LVCF_WIDTH, 0, smallWidth, _T("FPS"), 0, 3 },
      };

      for (int i = 0; i < _countof(columns); i++)
      {
         Verify(m_animations.InsertColumn(i, &columns[i]) == i);
      }

      {
         int iItem = m_animations.InsertItem(0, "All");
         if (iItem >= 0)
         {
            m_animations.SetItemText(iItem, 1, _T("0"));
            m_animations.SetItemText(iItem, 2, temp);
         }
      }
   }
   else
   {
      static const UINT disable[] = { IDC_EXPORTANIMATIONS, IDC_ANIMATIONS };
      for (int i = 0; i < _countof(disable); i++)
      {
         CWindow ctrl(GetDlgItem(disable[i]));
         if (ctrl.IsWindow())
            ctrl.EnableWindow(FALSE);
      }
   }

   return TRUE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}


LRESULT cMsPlugInExportDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
   EndDialog(IDOK);
   return 0;
}


LRESULT cMsPlugInExportDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & bHandled)
{
   EndDialog(IDCANCEL);
   return 0;
}

LRESULT cMsPlugInExportDlg::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
   if (ChooseExportFileName(&m_exportFileName))
   {
      DoDataExchange(FALSE);
   }
   return 0;
}

LRESULT cMsPlugInExportDlg::OnExportAnimations(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
   m_animations.EnableWindow(IsDlgButtonChecked(IDC_EXPORTANIMATIONS));
   return 0;
}

/*
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
*/
