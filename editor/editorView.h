/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORVIEW_H)
#define INCLUDED_EDITORVIEW_H

#include "editorapi.h"
#include "afxcomtools.h"
#include "DynamicLink.h"

#include "matrix4.h"

#ifdef HAVE_DIRECTX
#include <d3d9.h>
#include <d3dx9.h>
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cEditorDoc;

F_DECLARE_INTERFACE(IDirect3D9);
F_DECLARE_INTERFACE(IDirect3DDevice9);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorView
//

class cEditorView : public CScrollView,
                    public cComObject3<IMPLEMENTS(IEditorView),
                                       IMPLEMENTS(IEditorAppListener),
                                       IMPLEMENTS(IEditorLoopClient),
                                       cAfxComServices<cEditorView> >
{
protected: // create from serialization only
	cEditorView();
	DECLARE_DYNCREATE(cEditorView)

// Attributes
public:
	cEditorDoc * GetDocument();

// Operations
public:
   bool Initialize();

   // IEditorView
   virtual tVec3 GetCameraEyePosition() const;
   virtual tResult GetCameraPlacement(float * px, float * pz);
   virtual tResult PlaceCamera(float x, float z);
   virtual tResult GetCameraElevation(float * pElevation);
   virtual tResult SetCameraElevation(float elevation);
   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay);
   virtual tResult GetModel(IEditorModel * * ppModel);
   virtual tResult HighlightTerrainQuad(HTERRAINQUAD hQuad);
   virtual tResult HighlightTerrainVertex(HTERRAINVERTEX hVertex);
   virtual tResult ClearHighlight();

   // IEditorAppListener
   virtual tResult OnActiveToolChange(IEditorTool * pNewTool, IEditorTool * pFormerTool);
   virtual tResult OnDefaultTileSetChange(const tChar * pszTileSet);

   // IEditorLoopClient
   virtual void OnFrame(double time, double elapsed);

   void RenderGL();
   void RenderD3D();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
   virtual void OnFinalRelease();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~cEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(cEditorView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   bool InitGL();
   bool InitD3D();

   void UpdateCompositeMatrices();

private:
   bool m_bInitialized;
   bool m_bUsingD3d;

   HDC m_hDC;
   HGLRC	m_hRC;

#ifdef HAVE_DIRECTX
   cDynamicLink m_d3d9Lib;
   cAutoIPtr<IDirect3D9> m_pD3d;
   cAutoIPtr<IDirect3DDevice9> m_pD3dDevice;
   D3DPRESENT_PARAMETERS m_presentParams;
#endif

   float m_cameraElevation;
   tVec3 m_center;
   mutable tVec3 m_eye;
   mutable bool m_bRecalcEye; // recalculate camera eye position when placement/elevation changes

   tMatrix4 m_proj, m_view;
   tMatrix4 m_viewInverse, m_viewProj, m_viewProjInverse;
   bool m_bUpdateCompositeMatrices;

   HTERRAINQUAD m_highlightQuad;
   HTERRAINVERTEX m_highlightVertex;

   bool m_bInPostNcDestroy;
};

#ifndef _DEBUG  // debug version in editorView.cpp
inline cEditorDoc* cEditorView::GetDocument()
   { return (cEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORVIEW_H)
