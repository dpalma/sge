/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORVIEW_H)
#define INCLUDED_EDITORVIEW_H

#include "editorapi.h"
#include "GLContext.h"

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

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorView
//

extern const uint WM_GET_IEDITORVIEW;

typedef CWinTraitsOR<0, WS_EX_CLIENTEDGE> tEditorViewTraits;

class cEditorView : public CComObjectRoot,
                    public CComCoClass<cEditorView, &CLSID_EditorView>,
                    public CWindowImpl<cEditorView, CWindow, tEditorViewTraits>,
                    public cGLContext<cEditorView>,
                    public cConnectionPoint<IWindow, IWindowSink>,
                    public IEditorView,
                    public IEditorLoopClient
{
   typedef cGLContext<cEditorView> tGLBase;
   typedef CWindowImpl<cEditorView, CWindow, tEditorViewTraits> tWindowImplBase;

public:
	cEditorView();
	~cEditorView();

   DECLARE_WND_CLASS("EditorView")

   DECLARE_NO_REGISTRY()
   DECLARE_NOT_AGGREGATABLE(cEditorView)

   BEGIN_COM_MAP(cEditorView)
      COM_INTERFACE_ENTRY_IID(IID_IWindow, IWindow)
      COM_INTERFACE_ENTRY(IEditorView)
      COM_INTERFACE_ENTRY(IEditorLoopClient)
   END_COM_MAP()

   IRenderDevice * AccessRenderDevice();

   // IWindow
   virtual tResult Create(const sWindowCreateParams * pParams);
   virtual tResult SwapBuffers();

   // IEditorView
   virtual tResult Create(HWND hWndParent, HWND * phWnd);
   virtual tResult Destroy();
   virtual tResult Move(int x, int y, int width, int height);
   virtual tResult GetCamera(ISceneCamera * * ppCamera);
   virtual tVec3 GetCameraEyePosition() const;
   virtual tResult GetCameraPlacement(float * px, float * pz);
   virtual tResult PlaceCamera(float x, float z);
   virtual tResult GetCameraElevation(float * pElevation);
   virtual tResult SetCameraElevation(float elevation);
   virtual tResult GetModel(IEditorModel * * ppModel);
   virtual tResult SetModel(IEditorModel * pModel);
   virtual tResult HighlightTile(int iTileX, int iTileZ);
   virtual tResult ClearTileHighlight();

   // IEditorLoopClient
   virtual void OnFrame(double time, double elapsed);

   void RenderScene();

   BEGIN_MSG_MAP_EX(cEditorView)
      CHAIN_MSG_MAP(tGLBase)
	   MSG_WM_CREATE(OnCreate)
	   MSG_WM_DESTROY(OnDestroy)
	   MSG_WM_SIZE(OnSize)
      MSG_WM_PAINT(OnPaint)
      MESSAGE_HANDLER(WM_GET_IEDITORVIEW, OnGetIEditorView)
   END_MSG_MAP()

   LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
   void OnDestroy();
   void OnSize(UINT nType, CSize size);
   void OnPaint(HDC hDc);
   LRESULT OnGetIEditorView(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorView)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	void InitialUpdate();
	void Update();

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

private:
   cAutoIPtr<IEditorModel> m_pModel;

   cAutoIPtr<IRenderDevice> m_pRenderDevice;

   float m_cameraElevation;
   tVec3 m_center;
   mutable tVec3 m_eye;
   mutable bool m_bRecalcEye; // recalculate camera eye position when placement/elevation changes
   cAutoIPtr<ISceneCamera> m_pCamera;

   cSceneEntity m_sceneEntity;

   cAutoIPtr<IVertexBuffer> m_pVertexBuffer;
   cAutoIPtr<IIndexBuffer> m_pIndexBuffer;
   uint m_nIndices;

   int m_highlitTileX, m_highlitTileZ;
};

////////////////////////////////////////

inline IRenderDevice * cEditorView::AccessRenderDevice()
{
   return m_pRenderDevice;
}

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORVIEW_H)
