/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORVIEW_H)
#define INCLUDED_EDITORVIEW_H

#include "glView.h"
#include "editorapi.h"
#include "afxcomtools.h"

#include "matrix4.h"
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

class cEditorView : public cGLView,
                    public cComObject3<IMPLEMENTS(IWindow),
                                       IMPLEMENTS(IEditorView),
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
   inline IRenderDevice * AccessRenderDevice() { return m_pRenderDevice; }

   // IWindow
   virtual tResult Connect(IWindowSink *) { Assert(!"DON'T CALL THIS!"); return E_NOTIMPL; }
   virtual tResult Disconnect(IWindowSink *) { Assert(!"DON'T CALL THIS!"); return E_NOTIMPL; }
   virtual tResult Create(const sWindowCreateParams * pParams);
   virtual tResult SwapBuffers();

   // IEditorView
   virtual tResult GetCamera(ISceneCamera * * ppCamera);
   virtual tVec3 GetCameraEyePosition() const;
   virtual tResult GetCameraPlacement(float * px, float * pz);
   virtual tResult PlaceCamera(float x, float z);
   virtual tResult GetCameraElevation(float * pElevation);
   virtual tResult SetCameraElevation(float elevation);
   virtual tResult GetModel(IEditorModel * * ppModel);
   virtual tResult GetHighlightTile(int * piTileX, int * piTileZ) const;
   virtual tResult HighlightTile(int iTileX, int iTileZ);
   virtual tResult ClearTileHighlight();

   // IEditorLoopClient
   virtual void OnFrame(double time, double elapsed);

   void RenderScene();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorView)
	public:
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   cAutoIPtr<IRenderDevice> m_pRenderDevice;

   float m_cameraElevation;
   tVec3 m_center;
   mutable tVec3 m_eye;
   mutable bool m_bRecalcEye; // recalculate camera eye position when placement/elevation changes
   cAutoIPtr<ISceneCamera> m_pCamera;

   int m_highlitTileX, m_highlitTileZ;

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
