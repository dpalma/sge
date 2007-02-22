///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTEXT_H
#define INCLUDED_GUICONTEXT_H

#include "gui/guiapi.h"
#include "gui/guielementapi.h"
#include "guieventrouter.h"

#include "platform/inputapi.h"
#include "script/scriptapi.h"
#include "tech/globalobjdef.h"

#include <list>

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
                                       IMPLEMENTS(IGlobalObject),
                                       IMPLEMENTS(IScriptable)>
{
   enum eGUIPagePlane
   {
      kPages, kDialogs, kOverlays
   };

public:
   cGUIContext(const tChar * pszScriptName);
   ~cGUIContext();

   DECLARE_NAME(GUIContext)
   DECLARE_CONSTRAINTS()

   // IGlobalObject methods
   virtual tResult Init();
   virtual tResult Term();

   // IScriptable methods
   virtual tResult Invoke(const char * pszMethodName,
                          int argc, const tScriptVar * argv,
                          int nMaxResults, tScriptVar * pResults);

   tResult InvokeShowModalDialog(int argc, const tScriptVar * argv,
                                 int nMaxResults, tScriptVar * pResults);
   tResult InvokeClear(int argc, const tScriptVar * argv,
                       int nMaxResults, tScriptVar * pResults);
   tResult InvokePushPage(int argc, const tScriptVar * argv,
                          int nMaxResults, tScriptVar * pResults);
   tResult InvokePopPage(int argc, const tScriptVar * argv,
                         int nMaxResults, tScriptVar * pResults);
   tResult InvokeToggleDebugInfo(int argc, const tScriptVar * argv,
                                 int nMaxResults, tScriptVar * pResults);
   tResult InvokeGetElement(int argc, const tScriptVar * argv,
                            int nMaxResults, tScriptVar * pResults);
   tResult InvokeAddOverlay(int argc, const tScriptVar * argv,
                            int nMaxResults, tScriptVar * pResults);

   // IGUIContext methods
   virtual tResult ShowModalDialog(const tGUIChar * pszDialog);

   tResult PushPage(eGUIPagePlane plane, cGUIPage * pPage);
   tResult PopPage(eGUIPagePlane plane);

   virtual tResult PushPage(const tGUIChar * pszPage);
   virtual tResult PopPage();

   virtual tResult AddOverlayPage(const tGUIChar * pszPage);

   virtual tResult GetElementById(const tGUIChar * pszId, IGUIElement * * ppElement);

   virtual tResult GetOverlayElement(const tGUIChar * pszId, IGUIElement * * ppElement);

   virtual tResult RequestLayout(IGUIElement * pRequester, uint options);

   virtual tResult RenderGUI(uint width, uint height, IRender2D * pRender2D);

   virtual tResult ShowDebugInfo(const tGUIPoint & placement, IGUIStyle * pStyle);
   virtual tResult HideDebugInfo();

   virtual tResult GetDefaultFont(IRenderFont * * ppFont);

   tResult GetHitElement(const tScreenPoint & point, IGUIElement * * ppElement) const;

   cGUIPage * GetCurrentPage();
   const cGUIPage * GetCurrentPage() const;

private:
#ifdef GUI_DEBUG
   tResult GetDebugFont(IRenderFont * * ppFont);
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

   tGUIString m_scriptName;

#ifdef GUI_DEBUG
   bool m_bShowDebugInfo;
   tGUIPoint m_debugInfoPlacement;
   tGUIColor m_debugInfoTextColor;
   tScreenPoint m_lastMousePos;
   cAutoIPtr<IRenderFont> m_pDebugFont;
#endif

   typedef std::list<cGUIPage *> tGUIPageList;
   tGUIPageList m_pagePlanes[3];

   cAutoIPtr<IRenderFont> m_pDefaultFont;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUICONTEXT_H
