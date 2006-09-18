/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_IGVIEW_H)
#define INCLUDED_IGVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cImageGenDoc;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenView
//

class cImageGenView : public CScrollView
{
protected: // create from serialization only
	cImageGenView();
	DECLARE_DYNCREATE(cImageGenView)

// Attributes
public:
	cImageGenDoc * GetDocument();

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cImageGenView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~cImageGenView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(cImageGenView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
};

#ifndef _DEBUG  // debug version in editorView.cpp
inline cEditorDoc* cImageGenView::GetDocument()
   { return (cEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_IGVIEW_H)
