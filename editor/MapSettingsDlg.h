/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAPSETTINGSDLG_H)
#define INCLUDED_MAPSETTINGSDLG_H

#include "resource.h"       // main symbols

#include "str.h"

#include <atlddx.h>
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
                        public CWinDataExchange<cMapSettingsDlg>
{
public:
   enum { IDD = IDD_MAPSETTINGS };

   cMapSettingsDlg(const SIZE * pSizes, size_t nSizes, int sizeSelectIndex,
      const std::vector<cStr> & tileSets, int tileSetSelectIndex,
      eHeightData heightData);

   bool GetSelectedSize(SIZE * pSize) const;
   bool GetSelectedTileSet(cStr * pTileSet) const;
   eHeightData GetHeightData() const;
   bool GetHeightDataFile(cStr * pHeightData) const;

   virtual void OnCancel();

   BEGIN_MSG_MAP(cMapSettingsDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
   END_MSG_MAP()

   BEGIN_DDX_MAP(cMapSettingsDlg)
      DDX_CONTROL(IDC_MAP_INITIAL_TILE, m_initialTileComboBox);
//    DDX_CBIndex(pDX, IDC_MAP_TILESET, m_tileSetIndex);
      DDX_RADIO(IDC_HEIGHT_NONE, m_heightData);
      DDX_TEXT(IDC_HEIGHT_MAP_FILE, m_heightMapFile);
//    DDV_MaxChars(pDX, m_heightMapFile, MAX_PATH);
//    DDX_CBIndex(pDX, IDC_MAP_HEIGHT, m_mapHeightIndex);
//    DDX_CBIndex(pDX, IDC_MAP_WIDTH, m_mapWidthIndex);
   END_DDX_MAP()

// Implementation
protected:

   LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

   // Generated message map functions
   //{{AFX_MSG(cMapSettingsDlg)
   afx_msg void OnBrowseHeightMap();
   //}}AFX_MSG

   afx_msg void DoRadioButtonEnabling();
   afx_msg void OnSelectTileSet();

   void PopulateInitialTileComboBox(bool bForce);

private:
   CComboBoxEx m_initialTileComboBox;
   int      m_tileSetIndex;
   int      m_heightData;
   CString  m_heightMapFile;
   int      m_mapHeightIndex;
   int      m_mapWidthIndex;

   std::vector<SIZE> m_mapSizes;
   std::vector<cStr> m_tileSets;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MAPSETTINGSDLG_H)
