/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_CAMERADLG_H)
#define INCLUDED_CAMERADLG_H

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCameraDlg
//

class cCameraDlg : public CDialog
{
public:
	cCameraDlg(CWnd * pParentWnd);

   int m_fov, m_znear, m_zfar, m_elevation, m_pitch;

// Dialog Data
	//{{AFX_DATA(cCameraDlg)
	enum { IDD = IDD_CAMERA };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cCameraDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(cCameraDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_CAMERADLG_H)
