#include "stdafx.h"
#include "ExportPreviewDlg.h"
#include "resource.h"

#include "Exporter.h"

#include "techstring.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

static HTREEITEM InsertTreeItem(HWND hWnd, HTREEITEM hParent, const tChar * pszText)
{
   TVINSERTSTRUCT tvi = {0};
   tvi.hParent = hParent;
   tvi.hInsertAfter = TVI_LAST;
   tvi.item.mask = TVIF_TEXT;
   tvi.item.pszText = const_cast<char *>(pszText);
   return TreeView_InsertItem(hWnd, &tvi);
}

int_ptr CALLBACK ExportPreviewDlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg)
   {
      case WM_INITDIALOG:
      {
         SetWindowLongPtr(hWndDlg, GWL_USERDATA, lParam);

         cExporter * pExporter = (cExporter *)lParam;

         HWND hWndTree = GetDlgItem(hWndDlg, IDC_MODEL_PREVIEW_TREE);

         if (pExporter != NULL && hWndTree != NULL)
         {
            {
               std::vector<cExportMesh>::const_iterator iter = pExporter->BeginMeshes(), end = pExporter->EndMeshes();
               for (int index = 0; iter != end; ++iter, ++index)
               {
                  cStr temp;
                  HTREEITEM hItem = InsertTreeItem(hWndTree, TVI_ROOT, Sprintf(&temp, "Mesh %d", index).c_str());
                  if (hItem != NULL)
                  {
                     HTREEITEM hVerticesItem = InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "%d Vertices", iter->m_vertices.size()).c_str());
                     if (hVerticesItem != NULL)
                     {
                        for (uint i = 0; i < iter->m_vertices.size(); ++i)
                        {
                           const sModelVertex & v = iter->m_vertices[i];
                           InsertTreeItem(hWndTree, hVerticesItem, Sprintf(&temp, "UV: (%f, %f), Normal <%f, %f, %f>, Position (%f, %f, %f), Bone %f",
                              v.u, v.v,
                              v.normal.x, v.normal.y, v.normal.z,
                              v.pos.x, v.pos.y, v.pos.z,
                              v.bone).c_str());
                        }
                     }

                     static const tChar * primitives[] =
                     {
                        _T("Triangle List"),
                        _T("Triangle Strip"),
                        _T("Triangle Fan"),
                     };

                     HTREEITEM hIndicesItem = InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "%d Indices (%s)", iter->m_indices.size(), primitives[iter->m_primitive]).c_str());
                     if (hIndicesItem != NULL)
                     {
                        for (uint i = 0; i < iter->m_indices.size(); ++i)
                        {
                           InsertTreeItem(hWndTree, hIndicesItem, Sprintf(&temp, "%d", iter->m_indices[i]).c_str());
                        }
                     }
                  }
               }
            }

            {
               HTREEITEM hMaterialsItem = TVI_ROOT;
               std::vector<sModelMaterial>::const_iterator iter = pExporter->BeginMaterials(), end = pExporter->EndMaterials();
               for (int index = 0; iter != end; ++iter, ++index)
               {
                  cStr temp;
                  if (index == 0)
                  {
                     hMaterialsItem = InsertTreeItem(hWndTree, TVI_ROOT, "Materials");
                  }
                  HTREEITEM hItem = InsertTreeItem(hWndTree, hMaterialsItem, Sprintf(&temp, "Material %d", index).c_str());
                  if (hItem != NULL)
                  {
                     InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Diffuse: %f, %f, %f, %f", iter->diffuse[0], iter->diffuse[1], iter->diffuse[2], iter->diffuse[3]).c_str());
                     InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Ambient: %f, %f, %f, %f", iter->ambient[0], iter->ambient[1], iter->ambient[2], iter->ambient[3]).c_str());
                     InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Specular: %f, %f, %f, %f", iter->specular[0], iter->specular[1], iter->specular[2], iter->specular[3]).c_str());
                     InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Emissive: %f, %f, %f, %f", iter->emissive[0], iter->emissive[1], iter->emissive[2], iter->emissive[3]).c_str());
                     InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Shininess: %f", iter->shininess).c_str());
                     InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Texture: %s", iter->szTexture).c_str());
                  }
               }
            }

            {
               HTREEITEM hSkeletonItem = TVI_ROOT;
               std::vector<sModelJoint>::const_iterator iter = pExporter->BeginModelJoints(), end = pExporter->EndModelJoints();
               for (int index = 0; iter != end; ++iter, ++index)
               {
                  cStr temp;
                  if (index == 0)
                  {
                     hSkeletonItem = InsertTreeItem(hWndTree, TVI_ROOT, "Skeleton");
                  }
                  HTREEITEM hItem = InsertTreeItem(hWndTree, hSkeletonItem, Sprintf(&temp, "Joint %d", index).c_str());
                  if (hItem != NULL)
                  {
                     InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Parent: %d", iter->parentIndex).c_str());
                     InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Local Translation: %f, %f, %f", iter->localTranslation.x, iter->localTranslation.y, iter->localTranslation.z).c_str());
                     InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Local Rotation: %f, %f, %f, %f", iter->localRotation.x, iter->localRotation.y, iter->localRotation.z, iter->localRotation.w).c_str());
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
               std::vector<cExportAnimation>::const_iterator iter = pExporter->BeginAnimSeqs(), end = pExporter->EndAnimSeqs();
               for (int index = 0; iter != end; ++iter, ++index)
               {
                  cStr temp;
                  HTREEITEM hItem = InsertTreeItem(hWndTree, TVI_ROOT, Sprintf(&temp, "Animation Sequence %d (%s)", index, animationTypes[iter->GetAnimationType()]).c_str());
                  if (hItem != NULL)
                  {
                     std::vector<tModelKeyFrameVector>::const_iterator iter2 = iter->m_keyFrameVectors.begin();
                     std::vector<tModelKeyFrameVector>::const_iterator end2 = iter->m_keyFrameVectors.end();
                     for (int index2 = 0; iter2 != end2; ++iter2, ++index2)
                     {
                        HTREEITEM hItem2 = InsertTreeItem(hWndTree, hItem, Sprintf(&temp, "Joint %d KeyFrames (size = %d)", index2, iter2->size()).c_str());
                        std::vector<sModelKeyFrame>::const_iterator iter3 = iter2->begin();
                        std::vector<sModelKeyFrame>::const_iterator end3 = iter2->end();
                        for (; iter3 != end3; ++iter3)
                        {
                           const sModelKeyFrame & kf = *iter3;
                           InsertTreeItem(hWndTree, hItem2, Sprintf(&temp, "Time: %f, Translation <%f, %f, %f>, Rotation (%f, %f, %f, %f)",
                              kf.time,
                              kf.translation.x, kf.translation.y, kf.translation.z,
                              kf.rotation.x, kf.rotation.y, kf.rotation.z, kf.rotation.w).c_str());
                        }
                     }
                  }
               }
            }
         }

         return TRUE;
      }

      case WM_COMMAND:
      {
         if (HIWORD(wParam) == BN_CLICKED)
         {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
               EndDialog(hWndDlg, LOWORD(wParam));
            }
         }
         break;
      }

      case WM_SIZE:
      {
         // TODO
         break;
      }
   }

   return FALSE;
}

bool ShowExportPreviewDlg(cExporter * pExporter)
{
   if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_PREVIEW), NULL, ExportPreviewDlgProc, (LPARAM)pExporter) == IDOK)
   {
      return true;
   }

   return false;
}
