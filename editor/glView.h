/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_GLVIEW_H)
#define INCLUDED_GLVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGLView
//

class cGLView : public CScrollView
{
protected:
	cGLView();
   DECLARE_DYNAMIC(cGLView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cGLView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~cGLView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   HDC GetSafeHdc() const { return (this == NULL) ? NULL : m_hDC; }
   HGLRC GetSafeHglrc() const { return (this == NULL) ? NULL : m_hRC; }

// Generated message map functions
protected:
	//{{AFX_MSG(cGLView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   HDC m_hDC;
   HGLRC	m_hRC;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_GLVIEW_H)
