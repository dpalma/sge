/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAPSETTINGSDLG_H)
#define INCLUDED_MAPSETTINGSDLG_H

#include "ddxext.h"
#include "resource.h"       // main symbols

#include "str.h"

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
      COMMAND_ID_HANDLER(IDOK, OnOK)
      COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
   END_MSG_MAP()

   BEGIN_DDX_MAP(cMapSettingsDlg)
      DDX_CONTROL_HANDLE(IDC_MAP_INITIAL_TILE, m_initialTileComboBox);
      DDX_COMBO_INDEX(IDC_MAP_TILESET, m_tileSetIndex);
      DDX_RADIO(IDC_HEIGHT_NONE, m_heightData);
      DDX_TEXT(IDC_HEIGHT_MAP_FILE, m_heightMapFile);
      DDX_TEXT_LEN(IDC_HEIGHT_MAP_FILE, m_heightMapFile, MAX_PATH);
      DDX_COMBO_INDEX(IDC_MAP_HEIGHT, m_mapHeightIndex);
      DDX_COMBO_INDEX(IDC_MAP_WIDTH, m_mapWidthIndex);
   END_DDX_MAP()

// Implementation
protected:

   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & bHandled);

   // Generated message map functions
   //{{AFX_MSG(cMapSettingsDlg)
   void OnBrowseHeightMap();
   //}}AFX_MSG

   void DoRadioButtonEnabling();
   void OnSelectTileSet();

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
