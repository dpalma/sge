/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORVIEW_H)
#define INCLUDED_EDITORVIEW_H

#include "editorapi.h"
#include "afxcomtools.h"

#include "vec3.h"
#include "window.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IRenderDevice);

F_DECLARE_INTERFACE(ISceneCamera);

class cEditorDoc;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorView
//

class cEditorView : public CView,
                    public cComObject<IMPLEMENTS(IEditorLoopClient), cAfxComServices<cEditorView> >
{
protected: // create from serialization only
	cEditorView();
	DECLARE_DYNCREATE(cEditorView)

// Attributes
public:
	cEditorDoc * GetDocument();

// Operations
public:
   inline IRenderDevice * AccessRenderDevice() { return m_pRenderDevice; }

   virtual void OnFrame(double time, double elapsed);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~cEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(cEditorView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   HDC m_hDC;
   HGLRC	m_hRC;

   cAutoIPtr<IRenderDevice> m_pRenderDevice;

   tVec3 m_center, m_eye;
   cAutoIPtr<ISceneCamera> m_pCamera;
};

#ifndef _DEBUG  // debug version in editorView.cpp
inline cEditorDoc* cEditorView::GetDocument()
   { return (cEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORVIEW_H)
