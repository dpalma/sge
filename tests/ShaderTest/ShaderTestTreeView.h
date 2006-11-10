/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DTREEVIEW_H
#define INCLUDED_MS3DTREEVIEW_H

#include <afxcview.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cShaderTestDoc;
interface IModel;

/////////////////////////////////////////////////////////////////////////////
// cShaderTestTreeView view

class cShaderTestTreeView : public CTreeView
{
protected:
	cShaderTestTreeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(cShaderTestTreeView)

// Attributes
public:
	cShaderTestDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cShaderTestTreeView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~cShaderTestTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

   void AddVertices(IModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddMaterials(IModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddSubMeshes(IModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddSkeleton(IModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddBones(IModel * pModel, HTREEITEM hParent = TVI_ROOT);
   void AddAnimation(IModel * pModel, HTREEITEM hParent = TVI_ROOT);

	// Generated message map functions
protected:
	//{{AFX_MSG(cShaderTestTreeView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ms3dviewView.cpp
inline cShaderTestDoc* cShaderTestTreeView::GetDocument()
   { return (cShaderTestDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MS3DTREEVIEW_H)
