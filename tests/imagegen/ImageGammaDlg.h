/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_IMAGEGAMMADLG_H)
#define INCLUDED_IMAGEGAMMADLG_H

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cImageGammaDlg : public CDialog
{
	DECLARE_DYNAMIC(cImageGammaDlg)

public:
	cImageGammaDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~cImageGammaDlg();

// Dialog Data
	enum { IDD = IDD_IMAGE_GAMMA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
   BOOL m_bApplyGamma;
   float m_gamma;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_IMAGEGAMMADLG_H)
