/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAPSETTINGSDLG_H)
#define INCLUDED_MAPSETTINGSDLG_H

#include "resource.h"       // main symbols

#include "str.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapSettingsDlg
//

class cMapSettingsDlg : public CDialog
{
// Construction
public:
	cMapSettingsDlg(const SIZE * pSizes, size_t nSizes, int sizeSelectIndex,
      const std::vector<cStr> & tileSets, int tileSetSelectIndex,
      CWnd* pParent = NULL);   // standard constructor

   bool GetSelectedSize(SIZE * pSize);
   bool GetSelectedTileSet(cStr * pTileSet);

private:
// Dialog Data
	//{{AFX_DATA(cMapSettingsDlg)
	enum { IDD = IDD_MAPSETTINGS };
	int		m_mapSizeIndex;
	int		m_tileSetIndex;
	//}}AFX_DATA

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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   std::vector<SIZE> m_mapSizes;
   std::vector<cStr> m_tileSets;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MAPSETTINGSDLG_H)
