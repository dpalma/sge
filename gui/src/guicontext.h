///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTEXT_H
#define INCLUDED_GUICONTEXT_H

#include "guiapi.h"
#include "guieventrouter.h"

#include "inputapi.h"
#include "scriptapi.h"

#include "globalobjdef.h"

#ifdef _MSC_VER
#pragma once
#endif

#ifdef _DEBUG
#define GUI_DEBUG
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

class cGUIContext : public cComObject3<cGUIEventRouter<IGUIContext>, &IID_IGUIContext,
                                       IMPLEMENTS(IGlobalObject), IMPLEMENTS(IScriptable)>
{
public:
   cGUIContext();
   ~cGUIContext();

   DECLARE_NAME(GUIContext)
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult Invoke(const char * pszMethodName,
                          int argc, const cScriptVar * argv,
                          int nMaxResults, cScriptVar * pResults);

   tResult InvokeShowModalDialog(int argc, const cScriptVar * argv,
                                 int nMaxResults, cScriptVar * pResults);
   tResult InvokeClear(int argc, const cScriptVar * argv,
                       int nMaxResults, cScriptVar * pResults);
   tResult InvokeLoad(int argc, const cScriptVar * argv,
                      int nMaxResults, cScriptVar * pResults);
   tResult InvokeToggleDebugInfo(int argc, const cScriptVar * argv,
                                 int nMaxResults, cScriptVar * pResults);

   virtual tResult ShowModalDialog(const tChar * pszDialog);

   virtual tResult LoadElements(const char * pszXmlStringOrFile, bool bVisible);

   virtual void ClearGUI();

   virtual tResult RenderGUI();

   virtual tResult GetRenderDeviceContext(IGUIRenderDeviceContext * * ppRenderDeviceContext);
   virtual tResult SetRenderDeviceContext(IGUIRenderDeviceContext * pRenderDeviceContext);

   virtual tResult GetDefaultFont(IGUIFont * * ppFont);

   virtual tResult ShowDebugInfo(const tGUIPoint & placement, const tGUIColor & textColor);
   virtual tResult HideDebugInfo();

private:
   static tResult CheckModalDialog(IGUIElement * pElement);
   static tResult CheckChild(IGUIContainerElement * pContainer, const tChar * pszId, REFGUID iid);

#ifdef GUI_DEBUG
   tResult GetDebugFont(IGUIFont * * ppFont);
   void RenderDebugInfo();

   // Over-riding the cGUIEventRouter method even though it isn't virtual.
   // OK because it is only called through a cGUIContext* pointer in 
   // cInputListener::OnInputEvent
   bool HandleInputEvent(const sInputEvent * pEvent);
#endif

   class cInputListener : public cComObject<IMPLEMENTS(IInputListener)>
   {
      cGUIContext * m_pOuter;
   public:
      cInputListener(cGUIContext * pOuter);
      virtual bool OnInputEvent(const sInputEvent * pEvent);
   };

   friend class cInputListener;
   cInputListener m_inputListener;

   uint m_nElementsLastLayout; // How many elements were there the last time they were laid out?
   bool m_bNeedsLayout; // Set to true to force re-doing the layout

   bool m_bShowingModalDialog;

#ifdef GUI_DEBUG
   bool m_bShowDebugInfo;
   tGUIPoint m_debugInfoPlacement;
   tGUIColor m_debugInfoTextColor;
   tGUIPoint m_lastMousePos;
   cAutoIPtr<IGUIFont> m_pDebugFont;
#endif

   cAutoIPtr<IGUIRenderDeviceContext> m_pRenderDeviceContext;

   cAutoIPtr<IGUIFont> m_pDefaultFont;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUICONTEXT_H
