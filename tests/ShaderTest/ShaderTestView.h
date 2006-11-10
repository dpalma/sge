/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_SHADERTESTVIEW_H)
#define INCLUDED_SHADERTESTVIEW_H

#include "tech/comtools.h"
#include "tech/vec3.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cShaderTestDoc;

/////////////////////////////////////////////////////////////////////////////

class cShaderTestView : public CView, public ShaderTest::cFrameLoopClient
{
protected: // create from serialization only
	cShaderTestView();
	DECLARE_DYNCREATE(cShaderTestView)

// Attributes
public:
	cShaderTestDoc* GetDocument();

// Operations
public:
   virtual void OnFrame(double time, double elapsed);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cShaderTestView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~cShaderTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(cShaderTestView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   HDC m_hDC;
   HGLRC	m_hRC;

   tVec3 m_center, m_eye;
};

#ifndef _DEBUG  // debug version in ShaderTestView.cpp
inline cShaderTestDoc* cShaderTestView::GetDocument()
   { return (cShaderTestDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_SHADERTESTVIEW_H)
