/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MSPLUGINEXPORTDLG_H
#define INCLUDED_MSPLUGINEXPORTDLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMsPlugInExportDlg
//

class cMsPlugInExportDlg : public CDialog
{
// Construction
public:
	cMsPlugInExportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(cMsPlugInExportDlg)
	enum { IDD = IDD_EXPORT };
	CListCtrl	m_animations;
	BOOL	m_bExportAnimations;
	BOOL	m_bExportMaterials;
	CString	m_exportFileName;
	CString	m_skeletonFileName;
	//}}AFX_DATA
	CBitmapButton	m_addAnimationButton;
	CBitmapButton	m_removeAnimationButton;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cMsPlugInExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(cMsPlugInExportDlg)
	afx_msg void OnBrowse();
	afx_msg void OnBrowseSkeleton();
	afx_msg void OnAddAnimation();
	afx_msg void OnRemoveAnimation();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !INCLUDED_MSPLUGINEXPORTDLG_H
