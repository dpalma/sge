///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTEXT_H
#define INCLUDED_GUICONTEXT_H

#include "guiapi.h"
#include "guieventrouter.h"

#include "inputapi.h"
#include "scriptapi.h"

#include "globalobjdef.h"

#include <list>
#include <map>

#ifdef _MSC_VER
#pragma once
#endif

#ifdef _DEBUG
#define GUI_DEBUG
#endif

class cGUIPage;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

class cGUIContext : public cComObject3<cGUIEventRouter<cGUIContext, IGUIContext>, &IID_IGUIContext,
                                       IMPLEMENTS(IGlobalObject), IMPLEMENTS(IScriptable)>
{
public:
   cGUIContext();
   ~cGUIContext();

   DECLARE_NAME(GUIContext)
   DECLARE_CONSTRAINTS()

   // IGlobalObject methods
   virtual tResult Init();
   virtual tResult Term();

   // IScriptable methods
   virtual tResult Invoke(const char * pszMethodName,
                          int argc, const cScriptVar * argv,
                          int nMaxResults, cScriptVar * pResults);

   tResult InvokeShowModalDialog(int argc, const cScriptVar * argv,
                                 int nMaxResults, cScriptVar * pResults);
   tResult InvokeClear(int argc, const cScriptVar * argv,
                       int nMaxResults, cScriptVar * pResults);
   tResult InvokePushPage(int argc, const cScriptVar * argv,
                          int nMaxResults, cScriptVar * pResults);
   tResult InvokePopPage(int argc, const cScriptVar * argv,
                         int nMaxResults, cScriptVar * pResults);
   tResult InvokeToggleDebugInfo(int argc, const cScriptVar * argv,
                                 int nMaxResults, cScriptVar * pResults);
   tResult InvokeGetElement(int argc, const cScriptVar * argv,
                            int nMaxResults, cScriptVar * pResults);

   // IGUIContext methods
   virtual tResult ShowModalDialog(const tChar * pszDialog);

   virtual tResult PushPage(const tChar * pszPage);
   virtual tResult PopPage();

   virtual tResult GetElementById(const tChar * pszId, IGUIElement * * ppElement);

   virtual tResult RenderGUI();

   virtual tResult GetRenderDeviceContext(IGUIRenderDeviceContext * * ppRenderDeviceContext);
   virtual tResult SetRenderDeviceContext(IGUIRenderDeviceContext * pRenderDeviceContext);

   virtual tResult GetDefaultFont(IGUIFont * * ppFont);

   virtual tResult ShowDebugInfo(const tGUIPoint & placement, IGUIStyle * pStyle);
   virtual tResult HideDebugInfo();

   tResult GetHitElement(const tGUIPoint & point, IGUIElement * * ppElement) const;
   tResult GetActiveModalDialog(IGUIDialogElement * * ppModalDialog);

   cGUIPage * GetCurrentPage() { return m_pages.empty() ? NULL : m_pages.back(); }
   const cGUIPage * GetCurrentPage() const { return m_pages.empty() ? NULL : m_pages.back(); }

private:
   void ClearTempElementMap();

   tResult CheckDialogPage();

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

   std::list<cGUIPage *> m_pages;

   std::map<tGUIString, IGUIElement*> m_tempElementMap;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUICONTEXT_H
