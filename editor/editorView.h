/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORVIEW_H)
#define INCLUDED_EDITORVIEW_H

#include "glView.h"
#include "editorapi.h"
#include "afxcomtools.h"

#include "sceneapi.h"

#include "vec3.h"
#include "quat.h"
#include "matrix4.h"
#include "window.h"
#include "connptimpl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IVertexDeclaration);
F_DECLARE_INTERFACE(IVertexBuffer);
F_DECLARE_INTERFACE(IIndexBuffer);

F_DECLARE_INTERFACE(ISceneCamera);

class cEditorDoc;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorView
//

class cEditorView : public cGLView,
                    public cComObject2<IMPLEMENTSCP(IWindow, IWindowSink),
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

   virtual tResult Create(int width, int height, int bpp, const char * pszTitle = NULL);
   virtual tResult GetWindowInfo(sWindowInfo * pInfo) const;
   virtual tResult SwapBuffers();

   virtual void OnFrame(double time, double elapsed);

   void RenderScene();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~cEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   class cSceneEntity : public cComObject<IMPLEMENTS(ISceneEntity)>
   {
   public:
      cSceneEntity(cEditorView * pOuter);
      virtual ~cSceneEntity();

      virtual ISceneEntity * AccessParent() { return NULL; }
      virtual tResult SetParent(ISceneEntity * pEntity) { return E_FAIL; }
      virtual tResult IsChild(ISceneEntity * pEntity) const { return E_FAIL; }
      virtual tResult AddChild(ISceneEntity * pEntity) { return E_FAIL; }
      virtual tResult RemoveChild(ISceneEntity * pEntity) { return E_FAIL; }

      virtual const tVec3 & GetLocalTranslation() const { return m_translation; }
      virtual void SetLocalTranslation(const tVec3 & translation) { m_translation = translation; }
      virtual const tQuat & GetLocalRotation() const { return m_rotation; }
      virtual void SetLocalRotation(const tQuat & rotation) { m_rotation = rotation; }
      virtual const tMatrix4 & GetLocalTransform() const { return m_transform; }

      virtual const tVec3 & GetWorldTranslation() const { return GetLocalTranslation(); }
      virtual const tQuat & GetWorldRotation() const { return GetLocalRotation(); }
      virtual const tMatrix4 & GetWorldTransform() const { return GetLocalTransform(); }

      virtual void Render(IRenderDevice * pRenderDevice);
      virtual float GetBoundingRadius() const { return 9999999; }

      virtual tResult Intersects(const cRay & ray) { return E_NOTIMPL; }

   private:
      cEditorView * m_pOuter;
      tVec3 m_translation;
      tQuat m_rotation;
      tMatrix4 m_transform;
   };
   friend class cSceneEntity;

// Generated message map functions
protected:
	//{{AFX_MSG(cEditorView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
   enum eMouseAction
   {
      kNone,
      kMoveCamera,
   };

   eMouseAction m_mouseAction;

   CPoint m_lastMousePoint;

   cAutoIPtr<IRenderDevice> m_pRenderDevice;

   tVec3 m_center, m_eye;
   cAutoIPtr<ISceneCamera> m_pCamera;

   cSceneEntity m_sceneEntity;

   cAutoIPtr<IVertexBuffer> m_pVertexBuffer;
   cAutoIPtr<IIndexBuffer> m_pIndexBuffer;
   uint m_nIndices;
};

#ifndef _DEBUG  // debug version in editorView.cpp
inline cEditorDoc* cEditorView::GetDocument()
   { return (cEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORVIEW_H)
