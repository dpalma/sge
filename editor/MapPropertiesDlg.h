/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAPPROPERTIESDLG_H)
#define INCLUDED_MAPPROPERTIESDLG_H

#include "resource.h"       // main symbols

#include "tech/techstring.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMapPropertiesDlg
//
// The difference between map "properties" and map "settings" is that
// properties may be changed at any time, while settings are specified
// when the map is first created and may never be changed after that.

class cMapPropertiesDlg : public CDialog
{
// Construction
public:
	cMapPropertiesDlg(CWnd* pParent = NULL);   // standard constructor

   void SetTitle(LPCTSTR pszTitle);
   void GetTitle(CString * pTitle) const;
   void SetAuthor(LPCTSTR pszAuthor);
   void GetAuthor(CString * pAuthor) const;
   void SetNumPlayers(int numPlayers);
   int GetNumPlayers() const;
   void SetDescription(LPCTSTR pszDesc);
   void GetDescription(CString * pDesc) const;

private:
// Dialog Data
	//{{AFX_DATA(cMapPropertiesDlg)
	enum { IDD = IDD_MAPPROPERTIES };
	CString	m_title;
	CString	m_author;
	int		m_numPlayers;
	CSpinButtonCtrl	m_numPlayersSpin;
   CString m_description;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cMapPropertiesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(cMapPropertiesDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MAPPROPERTIESDLG_H)
