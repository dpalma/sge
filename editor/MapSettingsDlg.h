/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAPSETTINGSDLG_H)
#define INCLUDED_MAPSETTINGSDLG_H

#include "ddxext.h"
#include "resource.h"       // main symbols

#include "str.h"

#include <atlframe.h>

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum eHeightData;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettingsDlg
//

class cMapSettingsDlg : public CDialogImpl<cMapSettingsDlg>,
                        public CDialogResize<cMapSettingsDlg>,
                        public cWinDataExchangeEx<cMapSettingsDlg>
{
public:
   enum { IDD = IDD_MAPSETTINGS, kNoIndex = ~0 };

   cMapSettingsDlg(const SIZE * pSizes, size_t nSizes, int sizeSelectIndex,
      const std::vector<cStr> & tileSets, int tileSetSelectIndex,
      eHeightData heightData);

   bool GetSelectedSize(SIZE * pSize) const;
   bool GetSelectedTileSet(cStr * pTileSet) const;
   eHeightData GetHeightData() const;
   bool GetHeightDataFile(cStr * pHeightData) const;

   BEGIN_MSG_MAP(cMapSettingsDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDC_HEIGHT_NONE, DoRadioButtonEnabling)
      COMMAND_ID_HANDLER(IDC_HEIGHT_NOISE, DoRadioButtonEnabling)
      COMMAND_ID_HANDLER(IDC_IMPORT_HEIGHT_MAP, DoRadioButtonEnabling)
      COMMAND_ID_HANDLER(IDC_MAP_TILESET, OnSelectTileSet)
      COMMAND_ID_HANDLER(IDC_BROWSE_HEIGHT_MAP, OnBrowseHeightMap)
      COMMAND_ID_HANDLER(IDOK, OnOK)
      COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
      CHAIN_MSG_MAP(CDialogResize<cMapSettingsDlg>)
   END_MSG_MAP()

   BEGIN_DLGRESIZE_MAP(cMapSettingsDlg)
      DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_MAP_WIDTH, DLSZ_MOVE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_MAP_HEIGHT, DLSZ_MOVE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_MAP_TILESET, DLSZ_SIZE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_MAP_INITIAL_TILE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_HEIGHT_NONE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_HEIGHT_NOISE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_IMPORT_HEIGHT_MAP, DLSZ_SIZE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_HEIGHT_MAP_FILE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
      DLGRESIZE_CONTROL(IDC_BROWSE_HEIGHT_MAP, DLSZ_MOVE_X | DLSZ_MOVE_Y)
   END_DLGRESIZE_MAP()

   BEGIN_DDX_MAP(cMapSettingsDlg)
      DDX_CONTROL_HANDLE(IDC_MAP_INITIAL_TILE, m_initialTileComboBox);
      DDX_COMBO_INDEX(IDC_MAP_TILESET, m_tileSetIndex);
      DDX_RADIO(IDC_HEIGHT_NONE, m_heightData);
      DDX_TEXT(IDC_HEIGHT_MAP_FILE, m_heightMapFile);
      DDX_TEXT_LEN(IDC_HEIGHT_MAP_FILE, m_heightMapFile, MAX_PATH);
      DDX_COMBO_INDEX(IDC_MAP_HEIGHT, m_mapHeightIndex);
      DDX_COMBO_INDEX(IDC_MAP_WIDTH, m_mapWidthIndex);
   END_DDX_MAP()

   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT DoRadioButtonEnabling(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnSelectTileSet(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnBrowseHeightMap(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);

   void PopulateInitialTileComboBox(bool bForce);

private:
   CComboBoxEx m_initialTileComboBox;
   uint m_tileSetIndex;
   int m_heightData;
   CString  m_heightMapFile;
   uint m_mapHeightIndex;
   uint m_mapWidthIndex;

   std::vector<SIZE> m_mapSizes;
   std::vector<cStr> m_tileSets;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_MAPSETTINGSDLG_H)
