/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DTREEVIEW_H
#define INCLUDED_MS3DTREEVIEW_H

#include <afxcview.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class c3dmodelDoc;
class cModel;

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView view

class cMs3dTreeView : public CTreeView
{
protected:
	cMs3dTreeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(cMs3dTreeView)

// Attributes
public:
	c3dmodelDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cMs3dTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~cMs3dTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

   void AddVertices(cModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddMaterials(cModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddSubMeshes(cModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddSkeleton(cModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddBones(cModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddAnimation(cModel * pModel, HTREEITEM hParent = TVI_ROOT);

	// Generated message map functions
protected:
	//{{AFX_MSG(cMs3dTreeView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ms3dviewView.cpp
inline c3dmodelDoc* cMs3dTreeView::GetDocument()
   { return (c3dmodelDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MS3DTREEVIEW_H)
