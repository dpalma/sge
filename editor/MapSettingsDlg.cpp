/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MapSettingsDlg.h"
#include "editorapi.h"
#include "editorTypes.h"

#include "globalobj.h"

#include <atldlgs.h>

#include <algorithm>
#include <functional>

#include "dbgalloc.h" // must be last header

//////////////////////////////////////////////////////////////////////////////

static const SIZE g_mapSizes[] =
{
   { 64, 64 },
   { 128, 128 },
   { 192, 192 },
   { 256, 256 },
};

static const uint kDefaultMapSizeIndex = 0;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettingsDlg
//

////////////////////////////////////////

cMapSettingsDlg::cMapSettingsDlg(eHeightData heightData)
 :	m_heightData(heightData),
	m_heightMapFile(_T("")),
	m_mapHeightIndex(kDefaultMapSizeIndex),
	m_mapWidthIndex(kDefaultMapSizeIndex)
{
   UseGlobal(EditorTileManager);
   pEditorTileManager->GetDefaultTileSet(&m_tileSet);
}

////////////////////////////////////////

tResult cMapSettingsDlg::GetMapSettings(cMapSettings * pMapSettings) const
{
   if (pMapSettings == NULL)
   {
      return E_POINTER;
   }

   *pMapSettings = cMapSettings(
      g_mapSizes[m_mapWidthIndex].cx,
      g_mapSizes[m_mapHeightIndex].cy,
      m_tileSet,
      (eHeightData)m_heightData,
      m_heightMapFile);

   return S_OK;
}

////////////////////////////////////////

LRESULT cMapSettingsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
   DlgResize_Init();

   // Don't sort the drop-lists because selection is defined and
   // returned as indices into the lists given in the constructor
   Assert(::IsWindow(GetDlgItem(IDC_MAP_WIDTH)));
   Assert(::IsWindow(GetDlgItem(IDC_MAP_HEIGHT)));
   Assert(::IsWindow(GetDlgItem(IDC_MAP_TILESET)));
   CWindow(GetDlgItem(IDC_MAP_WIDTH)).ModifyStyle(CBS_SORT, 0);
   CWindow(GetDlgItem(IDC_MAP_HEIGHT)).ModifyStyle(CBS_SORT, 0);
   CWindow(GetDlgItem(IDC_MAP_TILESET)).ModifyStyle(CBS_SORT, 0);

   for (uint i = 0; i < _countof(g_mapSizes); i++)
   {
      CString str;
      str.Format("%d", g_mapSizes[i].cx);
      SendDlgItemMessage(IDC_MAP_WIDTH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
      str.Format("%d", g_mapSizes[i].cy);
      SendDlgItemMessage(IDC_MAP_HEIGHT, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
   }

   PopulateTileSetComboBox();

   // Update the dialog controls from the member variables
   // after populating drop-lists, etc.
   DoDataExchange(FALSE);

   BOOL unused;
   DoRadioButtonEnabling(0,0,NULL,unused);
   PopulateInitialTileComboBox();

   CenterWindow(GetParent());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

////////////////////////////////////////

LRESULT cMapSettingsDlg::DoRadioButtonEnabling(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
   Assert(::IsWindow(GetDlgItem(IDC_IMPORT_HEIGHT_MAP)));
   Assert(::IsWindow(GetDlgItem(IDC_BROWSE_HEIGHT_MAP)));
   Assert(::IsWindow(GetDlgItem(IDC_HEIGHT_MAP_FILE)));

   BOOL bEnable = (IsDlgButtonChecked(IDC_IMPORT_HEIGHT_MAP) == BST_CHECKED);
   ::EnableWindow(GetDlgItem(IDC_BROWSE_HEIGHT_MAP), bEnable);
   ::EnableWindow(GetDlgItem(IDC_HEIGHT_MAP_FILE), bEnable);

   return 0;
}

////////////////////////////////////////

LRESULT cMapSettingsDlg::OnSelectTileSet(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
   DoDataExchange(TRUE);
   PopulateInitialTileComboBox();
   return 0;
}

////////////////////////////////////////

LRESULT cMapSettingsDlg::OnBrowseHeightMap(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled)
{
   // Force the m_heightData member variable to match the state of the radio buttons
   DoDataExchange(TRUE);

   CString filter;
   Verify(filter.LoadString(IDS_HEIGHT_MAP_FILTER));
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);
   if (dlg.DoModal() == IDOK)
   {
      m_heightMapFile = dlg.m_szFileName;
      DoDataExchange(FALSE);
   }

   return 0;
}

////////////////////////////////////////

LRESULT cMapSettingsDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
   DoDataExchange(TRUE);
   EndDialog(IDOK);
   return 0;
}

////////////////////////////////////////

LRESULT cMapSettingsDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & bHandled)
{
   // Don't allow the dialog to be cancelled (no Cancel button, ESC key, etc.)
   bHandled = TRUE;
   return 0;
}

////////////////////////////////////////

void cMapSettingsDlg::PopulateTileSetComboBox()
{
   SendDlgItemMessage(IDC_MAP_TILESET, CB_RESETCONTENT);

   uint nTileSets = 0;
   UseGlobal(EditorTileManager);
   if (pEditorTileManager->GetTileSetCount(&nTileSets) == S_OK && nTileSets > 0)
   {
      for (uint i = 0; i < nTileSets; i++)
      {
         cStr name;
         cAutoIPtr<IEditorTileSet> pTileSet;
         if (pEditorTileManager->GetTileSet(i, &pTileSet) == S_OK
            && pTileSet->GetName(&name) == S_OK)
         {
            SendDlgItemMessage(IDC_MAP_TILESET, CB_ADDSTRING, 0, (LPARAM)name.c_str());
         }
         else
         {
            WarnMsg1("Error getting tile set %d\n", i);
         }
      }
   }
}

////////////////////////////////////////

void cMapSettingsDlg::PopulateInitialTileComboBox()
{
   int sel = SendDlgItemMessage(IDC_MAP_TILESET, CB_GETCURSEL);
   if (sel != CB_ERR)
   {
      m_initialTileComboBox.ResetContent();

      cAutoIPtr<IEditorTileSet> pTileSet;
      UseGlobal(EditorTileManager);
      if (pEditorTileManager->GetTileSet(m_tileSet, &pTileSet) == S_OK)
      {
         int imageSize = m_initialTileComboBox.GetItemHeight(0);

         HIMAGELIST hImageList = NULL;
         if (pTileSet->GetImageList(imageSize, &hImageList) == S_OK)
         {
            m_initialTileComboBox.SetImageList(hImageList);
         }

         uint nTiles = 0;
         if (pTileSet->GetTileCount(&nTiles) == S_OK)
         {
            for (uint i = 0; i < nTiles; i++)
            {
               cAutoIPtr<IEditorTile> pTile;
               if (pTileSet->GetTile(i, &pTile) == S_OK)
               {
                  cStr tileName;
                  Verify(pTile->GetName(&tileName) == S_OK);

                  COMBOBOXEXITEM item;
                  item.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
                  item.iItem = i;
                  item.pszText = const_cast<char *>(tileName.c_str());
                  item.cchTextMax = tileName.length();
                  item.iImage = i;
                  item.iSelectedImage = i;

                  Verify(m_initialTileComboBox.InsertItem(&item) != CB_ERR);
               }
            }

            m_initialTileComboBox.SetCurSel(0);
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////
