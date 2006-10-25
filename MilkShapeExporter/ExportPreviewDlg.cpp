/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"
#include "ExportPreviewDlg.h"

#include "Exporter.h"

#include "techstring.h"

#include <atlctrlx.h>

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

static HTREEITEM InsertTreeItem(HWND hWnd, HTREEITEM hParent, const tChar * pszText)
{
   TVINSERTSTRUCT tvi = {0};
   tvi.hParent = hParent;
   tvi.hInsertAfter = TVI_LAST;
   tvi.item.mask = TVIF_TEXT;
   tvi.item.pszText = const_cast<tChar *>(pszText);
   return TreeView_InsertItem(hWnd, &tvi);
}

static void FillExportPreviewTree(HWND hWndTree, cExporter * pExporter)
{
   if (pExporter != NULL && hWndTree != NULL)
   {
      {
         HTREEITEM hVerticesItem = InsertTreeItem(hWndTree, TVI_ROOT, _T("Vertices"));
         cStr temp;
         std::vector<sModelVertex>::const_iterator iter = pExporter->BeginVertices(), end = pExporter->EndVertices();
         for (int index = 0; iter != end; ++iter, ++index)
         {
            const sModelVertex & v = *iter;
            InsertTreeItem(hWndTree, hVerticesItem,
               Sprintf(&temp, _T("UV: (%f, %f), Normal <%f, %f, %f>, Position (%f, %f, %f), Bone %f"),
               v.u, v.v,
               v.normal.x, v.normal.y, v.normal.z,
               v.pos.x, v.pos.y, v.pos.z,
               v.bone).c_str());
         }
      }

      {
         HTREEITEM hIndicesItem = InsertTreeItem(hWndTree, TVI_ROOT, _T("Indices"));
         cStr temp;
         std::vector<uint16>::const_iterator iter = pExporter->BeginIndices(), end = pExporter->EndIndices();
         for (int index = 0; iter != end; ++iter, ++index)
         {
            InsertTreeItem(hWndTree, hIndicesItem, Sprintf(&temp, _T("%d"), *iter).c_str());
         }
      }

      {
         static const tChar * primitives[] =
         {
            _T("Lines"),
            _T("Line Strip"),
            _T("Triangle List"),
            _T("Triangle Strip"),
            _T("Triangle Fan")
         };

         cStr temp;
         HTREEITEM hMeshesItem = InsertTreeItem(hWndTree, TVI_ROOT, _T("Meshes"));
         std::vector<sModelMesh>::const_iterator iter = pExporter->BeginModelMeshes(), end = pExporter->EndModelMeshes();
         for (int index = 0; iter != end; ++iter, ++index)
         {
            const sModelMesh & m = *iter;
            InsertTreeItem(hWndTree, hMeshesItem,
               Sprintf(&temp, _T("Mesh %d: %s, Material %d, First Index %d, # Indices %d"),
                  index, primitives[m.primitive], m.materialIndex, m.indexStart, m.nIndices).c_str());
         }
      }

      {
         HTREEITEM hMaterialsItem = InsertTreeItem(hWndTree, TVI_ROOT, _T("Materials"));
         cStr temp;
         std::vector<sModelMaterial>::const_iterator iter = pExporter->BeginMaterials(), end = pExporter->EndMaterials();
         for (int index = 0; iter != end; ++iter, ++index)
         {
            HTREEITEM hItem = InsertTreeItem(hWndTree, hMaterialsItem, Sprintf(&temp, _T("Material %d"), index).c_str());
            if (hItem != NULL)
            {
               InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Diffuse: %f, %f, %f, %f"), iter->diffuse[0], iter->diffuse[1], iter->diffuse[2], iter->diffuse[3]).c_str());
               InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Ambient: %f, %f, %f, %f"), iter->ambient[0], iter->ambient[1], iter->ambient[2], iter->ambient[3]).c_str());
               InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Specular: %f, %f, %f, %f"), iter->specular[0], iter->specular[1], iter->specular[2], iter->specular[3]).c_str());
               InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Emissive: %f, %f, %f, %f"), iter->emissive[0], iter->emissive[1], iter->emissive[2], iter->emissive[3]).c_str());
               InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Shininess: %f"), iter->shininess).c_str());
               InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Texture: %s"), iter->szTexture).c_str());
            }
         }
      }

      {
         HTREEITEM hSkeletonItem = InsertTreeItem(hWndTree, TVI_ROOT, _T("Skeleton"));
         cStr temp;
         std::vector<sModelJoint>::const_iterator iter = pExporter->BeginModelJoints(), end = pExporter->EndModelJoints();
         for (int index = 0; iter != end; ++iter, ++index)
         {
            HTREEITEM hItem = InsertTreeItem(hWndTree, hSkeletonItem, Sprintf(&temp, _T("Joint %d"), index).c_str());
            if (hItem != NULL)
            {
               InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Parent: %d"), iter->parentIndex).c_str());
               InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Local Translation: %f, %f, %f"), iter->localTranslation.x, iter->localTranslation.y, iter->localTranslation.z).c_str());
               InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Local Rotation: %f, %f, %f, %f"), iter->localRotation.x, iter->localRotation.y, iter->localRotation.z, iter->localRotation.w).c_str());
            }
         }
      }

      static const char * const animationTypes[] =
      {
         "Walk",
         "Run",
         "Death",
         "Attack",
         "Damage",
         "Idle",
      };

      {
         HTREEITEM hAnimsItem = InsertTreeItem(hWndTree, TVI_ROOT, _T("Animation Sequences"));
         cStr temp;
         std::vector<cExportAnimation>::const_iterator iter = pExporter->BeginAnimSeqs(), end = pExporter->EndAnimSeqs();
         for (int index = 0; iter != end; ++iter, ++index)
         {
            HTREEITEM hItem = InsertTreeItem(hWndTree, hAnimsItem, Sprintf(&temp, _T("Animation Sequence %d (%s)"), index, animationTypes[iter->GetAnimationType()]).c_str());
            if (hItem != NULL)
            {
               std::vector<tModelKeyFrameVector>::const_iterator iter2 = iter->m_keyFrameVectors.begin();
               std::vector<tModelKeyFrameVector>::const_iterator end2 = iter->m_keyFrameVectors.end();
               for (int index2 = 0; iter2 != end2; ++iter2, ++index2)
               {
                  HTREEITEM hItem2 = InsertTreeItem(hWndTree, hItem, Sprintf(&temp, _T("Joint %d KeyFrames (size = %d)"), index2, iter2->size()).c_str());
                  std::vector<sModelKeyFrame>::const_iterator iter3 = iter2->begin();
                  std::vector<sModelKeyFrame>::const_iterator end3 = iter2->end();
                  for (; iter3 != end3; ++iter3)
                  {
                     const sModelKeyFrame & kf = *iter3;
                     InsertTreeItem(hWndTree, hItem2, Sprintf(&temp, _T("Time: %f, Translation <%f, %f, %f>, Rotation (%f, %f, %f, %f)"),
                        kf.time,
                        kf.translation.x, kf.translation.y, kf.translation.z,
                        kf.rotation.x, kf.rotation.y, kf.rotation.z, kf.rotation.w).c_str());
                  }
               }
            }
         }
      }
   }
}

bool ShowExportPreviewDlg(cExporter * pExporter)
{
   cExportPreviewDlg dlg;
   if (dlg.DoModal(GetActiveWindow(), (LPARAM)pExporter) == IDOK)
   {
      return true;
   }

   return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cExportPreviewDlg
//

////////////////////////////////////////

static BOOL CALLBACK CreateHyperLinks(HWND hWnd, LPARAM lParam)
{
   std::vector<WTL::CHyperLink*> * pHyperLinks = (std::vector<WTL::CHyperLink*>*)lParam;

   tChar szClassName[30];
   if (GetClassName(hWnd, szClassName, _countof(szClassName)) > 0)
   {
      if (_tcsicmp(szClassName, _T("static")) == 0)
      {
         tChar szText[1024];
         if (GetWindowText(hWnd, szText, _countof(szText)) > 0)
         {
            if (_tcsstr(szText, _T("http://")))
            {
               WTL::CHyperLink * pHyperLink = new WTL::CHyperLink;
               if (pHyperLink != NULL)
               {
                  if (pHyperLink->SubclassWindow(hWnd))
                  {
                     pHyperLinks->push_back(pHyperLink);
                  }
                  else
                  {
                     delete pHyperLink;
                  }
               }
            }
         }
      }
   }

   return TRUE;
}

////////////////////////////////////////

LRESULT cExportPreviewDlg::OnInitDialog(UINT, WPARAM, LPARAM lParam, BOOL &)
{
   DoDataExchange();

   CenterWindow(GetParent());

   EnumChildWindows(m_hWnd, CreateHyperLinks, (LPARAM)&m_hyperLinks);

	DlgResize_Init();

   cExporter * pExporter = (cExporter *)lParam;

   FillExportPreviewTree(m_modelPreviewTree.m_hWnd, pExporter);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

////////////////////////////////////////

LRESULT cExportPreviewDlg::OnDestroy(UINT, WPARAM, LPARAM, BOOL &)
{
   std::vector<WTL::CHyperLink*>::iterator iter = m_hyperLinks.begin();
   for (; iter != m_hyperLinks.end(); iter++)
   {
      (*iter)->UnsubclassWindow();
      delete (*iter);
   }
   m_hyperLinks.clear();

   return 0;
}

////////////////////////////////////////

LRESULT cExportPreviewDlg::OnCloseCommand(WORD, WORD id, HWND, BOOL &)
{
   EndDialog(id);
   return 0;
}


/////////////////////////////////////////////////////////////////////////////
