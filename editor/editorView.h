/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORVIEW_H)
#define INCLUDED_EDITORVIEW_H

#include "editorapi.h"
#include "GLContext.h"

#include "vec3.h"
#include "window.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IRenderDevice);

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
                    public IWindow,
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
   virtual tResult Connect(IWindowSink *) { return E_NOTIMPL; }
   virtual tResult Disconnect(IWindowSink *) { return E_NOTIMPL; }
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
   virtual tResult GetHighlightTile(int * piTileX, int * piTileZ) const;
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

	void InitialUpdate();

private:
   cAutoIPtr<IEditorModel> m_pModel;

   cAutoIPtr<IRenderDevice> m_pRenderDevice;

   float m_cameraElevation;
   tVec3 m_center;
   mutable tVec3 m_eye;
   mutable bool m_bRecalcEye; // recalculate camera eye position when placement/elevation changes
   cAutoIPtr<ISceneCamera> m_pCamera;

   int m_highlitTileX, m_highlitTileZ;
};

////////////////////////////////////////

inline IRenderDevice * cEditorView::AccessRenderDevice()
{
   return m_pRenderDevice;
}

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORVIEW_H)
