/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorToolState.h"
#include "editorTypes.h"

#include <algorithm>

#include <boost/bind.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace boost;


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorToolState
//

///////////////////////////////////////

cEditorToolState::cEditorToolState()
 : m_hCurrentToolWnd(NULL)
{
}

///////////////////////////////////////

cEditorToolState::~cEditorToolState()
{
}

///////////////////////////////////////

tResult cEditorToolState::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cEditorToolState::Term()
{
   return S_OK;
}

////////////////////////////////////////

tResult cEditorToolState::HandleMessage(MSG * pMsg) 
{
   if (pMsg == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IEditorTool> pTool;
   if (AccessToolCapture() != NULL)
   {
      pTool = CTAddRef(AccessToolCapture());
   }
   else if (AccessActiveTool() != NULL)
   {
      pTool = CTAddRef(AccessActiveTool());
   }

   if (!!pTool)
   {
      m_hCurrentToolWnd = pMsg->hwnd;

      tResult toolResult = S_EDITOR_TOOL_CONTINUE;

      switch (pMsg->message)
      {
         case WM_CANCELMODE:
         case WM_CAPTURECHANGED:
         {
            ReleaseToolCapture();
            break;
         }

         case WM_KEYDOWN:
         {
            toolResult = pTool->OnKeyDown(cEditorKeyEvent(pMsg->wParam, pMsg->lParam));

            if ((toolResult == S_EDITOR_TOOL_CONTINUE) && (pMsg->wParam == VK_ESCAPE))
            {
               SetActiveTool(NULL);
            }
            break;
         }

         case WM_KEYUP:
         {
            toolResult = pTool->OnKeyUp(cEditorKeyEvent(pMsg->wParam, pMsg->lParam));
            break;
         }

         case WM_LBUTTONDBLCLK:
         {
            toolResult = pTool->OnLButtonDblClk(cEditorMouseEvent(pMsg->wParam, pMsg->lParam));
            break;
         }

         case WM_LBUTTONDOWN:
         {
            toolResult = pTool->OnLButtonDown(cEditorMouseEvent(pMsg->wParam, pMsg->lParam));
            break;
         }

         case WM_LBUTTONUP:
         {
            toolResult = pTool->OnLButtonUp(cEditorMouseEvent(pMsg->wParam, pMsg->lParam));
            break;
         }

         case WM_RBUTTONDBLCLK:
         {
            toolResult = pTool->OnRButtonDblClk(cEditorMouseEvent(pMsg->wParam, pMsg->lParam));
            break;
         }

         case WM_RBUTTONDOWN:
         {
            toolResult = pTool->OnRButtonDown(cEditorMouseEvent(pMsg->wParam, pMsg->lParam));
            break;
         }

         case WM_RBUTTONUP:
         {
            toolResult = pTool->OnRButtonUp(cEditorMouseEvent(pMsg->wParam, pMsg->lParam));
            break;
         }

         case WM_MOUSEMOVE:
         {
            toolResult = pTool->OnMouseMove(cEditorMouseEvent(pMsg->wParam, pMsg->lParam));
            break;
         }

         case WM_MOUSEWHEEL:
         {
            toolResult = pTool->OnMouseWheel(cEditorMouseWheelEvent(pMsg->wParam, pMsg->lParam));
            break;
         }
      }

      m_hCurrentToolWnd = NULL;

      if (toolResult == S_EDITOR_TOOL_HANDLED)
      {
         return S_OK;
      }
   }

   return S_FALSE;
}

////////////////////////////////////////

tResult cEditorToolState::GetActiveTool(IEditorTool * * ppTool)
{
   return m_pActiveTool.GetPointer(ppTool);
}

////////////////////////////////////////

tResult cEditorToolState::SetActiveTool(IEditorTool * pTool)
{
   ForEachConnection(bind(&IEditorToolStateListener::OnActiveToolChange, _1, pTool, AccessActiveTool()));

   if (!!m_pActiveTool)
   {
      m_pActiveTool->Deactivate();
   }

   SafeRelease(m_pActiveTool);

   if (pTool != NULL)
   {
      pTool->Activate();
      m_pActiveTool = CTAddRef(pTool);
   }

   return S_OK;
}

////////////////////////////////////////

tResult cEditorToolState::GetToolCapture(IEditorTool * * ppTool)
{
   return m_pToolCapture.GetPointer(ppTool);
}

////////////////////////////////////////

tResult cEditorToolState::SetToolCapture(IEditorTool * pTool)
{
   if (pTool == NULL)
   {
      return E_POINTER;
   }

   if (!IsWindow(m_hCurrentToolWnd))
   {
      DebugMsg("No valid window handle in SetToolCapture\n");
      return E_FAIL;
   }

   ::SetCapture(m_hCurrentToolWnd);

   SafeRelease(m_pToolCapture);

   m_pToolCapture = CTAddRef(pTool);

   return S_OK;
}

////////////////////////////////////////

tResult cEditorToolState::ReleaseToolCapture()
{
   if (!m_pToolCapture)
   {
      return S_FALSE;
   }
   else
   {
      ReleaseCapture();
      SafeRelease(m_pToolCapture);
      return S_OK;
   }
}

///////////////////////////////////////

tResult EditorToolStateCreate()
{
   cAutoIPtr<IEditorToolState> pEditorToolState(static_cast<IEditorToolState*>(new cEditorToolState));
   if (!pEditorToolState)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IEditorToolState, pEditorToolState);
}

///////////////////////////////////////////////////////////////////////////////
