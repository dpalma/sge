/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_IMAGEATTRIBUTESDLG_H)
#define INCLUDED_IMAGEATTRIBUTESDLG_H

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageAttributesDlg
//

class cImageAttributesDlg : public CDialog
{
// Construction
public:
	cImageAttributesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(cImageAttributesDlg)
	enum { IDD = IDD_IMAGE_ATTRIBUTES };
   int m_pixelFormat;
   UINT m_width;
   UINT m_height;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cImageAttributesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(cImageAttributesDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_IMAGEATTRIBUTESDLG_H)
