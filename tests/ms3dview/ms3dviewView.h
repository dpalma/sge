// ms3dviewView.h : interface of the CMs3dviewView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MS3DVIEWVIEW_H__17A7D20E_9722_41CF_B129_799ABB1EC346__INCLUDED_)
#define AFX_MS3DVIEWVIEW_H__17A7D20E_9722_41CF_B129_799ABB1EC346__INCLUDED_

#include "vec3.h"
#include "window.h"
#include "ConnPtImpl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IRenderDevice);

class CMs3dviewView : public CView, public CIUnknownImpl<CMs3dviewView, IWindow>
{
protected: // create from serialization only
	CMs3dviewView();
	DECLARE_DYNCREATE(CMs3dviewView)

// Attributes
public:
	CMs3dviewDoc* GetDocument();

// Operations
public:
   tResult Connect(IWindowSink * pSink)
   {
		Assert(!"Not Supported");
      return E_NOTIMPL;
   }

   tResult Disconnect(IWindowSink * pSink)
   {
		Assert(!"Not Supported");
      return E_NOTIMPL;
   }

   virtual tResult Create(int width, int height, int bpp, const char * pszTitle = NULL);
   virtual tResult GetWindowInfo(sWindowInfo * pInfo) const;
   virtual tResult SwapBuffers();

   inline IRenderDevice * AccessRenderDevice() { return m_pRenderDevice; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMs3dviewView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMs3dviewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMs3dviewView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	DECLARE_INTERFACE_MAP()

private:
   HDC m_hDC;
   HGLRC	m_hRC;

   cAutoIPtr<IRenderDevice> m_pRenderDevice;

   CSliderCtrl m_slider;

   tVec3 m_center, m_eye;
};

#ifndef _DEBUG  // debug version in ms3dviewView.cpp
inline CMs3dviewDoc* CMs3dviewView::GetDocument()
   { return (CMs3dviewDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MS3DVIEWVIEW_H__17A7D20E_9722_41CF_B129_799ABB1EC346__INCLUDED_)
