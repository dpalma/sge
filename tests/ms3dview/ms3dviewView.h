/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(AFX_MS3DVIEWVIEW_H__17A7D20E_9722_41CF_B129_799ABB1EC346__INCLUDED_)
#define AFX_MS3DVIEWVIEW_H__17A7D20E_9722_41CF_B129_799ABB1EC346__INCLUDED_

#include "comtools.h"
#include "vec3.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class c3dmodelDoc;

/////////////////////////////////////////////////////////////////////////////

class c3dmodelView : public CView
{
protected: // create from serialization only
	c3dmodelView();
	DECLARE_DYNCREATE(c3dmodelView)

// Attributes
public:
	c3dmodelDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(c3dmodelView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~c3dmodelView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(c3dmodelView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   HDC m_hDC;
   HGLRC	m_hRC;

   CSliderCtrl m_slider;

   tVec3 m_center, m_eye;
};

#ifndef _DEBUG  // debug version in ms3dviewView.cpp
inline c3dmodelDoc* c3dmodelView::GetDocument()
   { return (c3dmodelDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MS3DVIEWVIEW_H__17A7D20E_9722_41CF_B129_799ABB1EC346__INCLUDED_)
