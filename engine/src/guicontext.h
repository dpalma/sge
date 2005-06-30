///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTEXT_H
#define INCLUDED_GUICONTEXT_H

#include "guiapi.h"
#include "guieventrouter.h"

#include "inputapi.h"

#include "globalobjdef.h"

#ifdef _MSC_VER
#pragma once
#endif

class TiXmlDocument;

#ifdef _DEBUG
#define GUI_DEBUG
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

class cGUIContext : public cComObject2<cGUIEventRouter<IGUIContext>, &IID_IGUIContext, IMPLEMENTS(IGlobalObject)>
{
   typedef cComObject2<cGUIEventRouter<IGUIContext>, &IID_IGUIContext, IMPLEMENTS(IGlobalObject)> tBaseClass;

public:
   cGUIContext();
   ~cGUIContext();

   DECLARE_NAME(GUIContext)
   DECLARE_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult AddElement(IGUIElement * pElement);
   virtual tResult RemoveElement(IGUIElement * pElement);

   virtual tResult LoadFromResource(const char * psz, bool bVisible);
   virtual tResult LoadFromString(const char * psz, bool bVisible);

   uint LoadFromTiXmlDoc(TiXmlDocument * pTiXmlDoc, bool bVisible);

   virtual void ClearGUI();

   virtual tResult RenderGUI();

   virtual tResult ShowDebugInfo(const tGUIPoint & placement, const tGUIColor & textColor);
   virtual tResult HideDebugInfo();

private:
#ifdef GUI_DEBUG
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

   bool m_bNeedLayout;

#ifdef GUI_DEBUG
   bool m_bShowDebugInfo;
   tGUIPoint m_debugInfoPlacement;
   tGUIColor m_debugInfoTextColor;
   tGUIPoint m_lastMousePos;
#endif
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUICONTEXT_H
