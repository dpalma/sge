/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAPSETTINGSDLG_H)
#define INCLUDED_MAPSETTINGSDLG_H

#include "resource.h"       // main symbols

#include "techstring.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cTerrainSettings;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettingsDlg
//

class cMapSettingsDlg : public CDialog
{
// Construction
public:
	cMapSettingsDlg(const cTerrainSettings & terrainSettings, CWnd* pParent = NULL);   // standard constructor

   tResult GetTerrainSettings(cTerrainSettings * pTS) const;

private:
// Dialog Data
	//{{AFX_DATA(cMapSettingsDlg)
	enum { IDD = IDD_MAPSETTINGS };
	CSpinButtonCtrl	m_heightScaleSpin;
	CStatic	m_heightScaleLabel;
	CComboBoxEx	m_initialTileComboBox;
	CString	m_tileSet;
	int		m_heightData;
	CString	m_heightMapFile;
	int		m_mapHeightIndex;
	int		m_mapWidthIndex;
	int		m_heightScale;
	//}}AFX_DATA
   int m_initialTile;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cMapSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

   virtual void OnCancel();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(cMapSettingsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseHeightMap();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   afx_msg void OnUpdateHeightImportControl(CCmdUI *pCmdUI);
   afx_msg void DoRadioButtonEnabling();
   afx_msg void OnSelectTileSet();

   void PopulateTileSetComboBox();
   void PopulateInitialTileComboBox();

private:
   std::vector<cStr> m_tileSets;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MAPSETTINGSDLG_H)
