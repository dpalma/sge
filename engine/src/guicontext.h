///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTEXT_H
#define INCLUDED_GUICONTEXT_H

#include "guiapi.h"
#include "guieventrouter.h"

#include "inputapi.h"

#include "globalobj.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

class cGUIContext : public cGlobalObject<cGUIEventRouter<IGUIContext>, &IID_IGUIContext>
{
   typedef cGlobalObject<cGUIEventRouter<IGUIContext>, &IID_IGUIContext> tBaseClass;

public:
   cGUIContext();
   ~cGUIContext();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult LoadFromResource(const char * psz);
   virtual tResult LoadFromString(const char * psz);

   virtual tResult RenderGUI(IRenderDevice * pRenderDevice);

   virtual tResult ShowDebugInfo(const tGUIPoint & placement, const tGUIColor & textColor);
   virtual tResult HideDebugInfo();

private:
#ifdef _DEBUG
   void RenderDebugInfo(IRenderDevice * pRenderDevice);

   // Over-riding the cGUIEventRouter method even though it isn't virtual.
   // OK because it is only called through a cGUIContext* pointer in 
   // cInputListener::OnInputEvent
   bool HandleInputEvent(const sInputEvent * pEvent);
#endif

   class cInputListener : public cComObject<IMPLEMENTS(IInputListener)>
   {
      virtual bool OnInputEvent(const sInputEvent * pEvent);
   };

   friend class cInputListener;
   cInputListener m_inputListener;

   bool m_bNeedLayout;

#ifdef _DEBUG
   bool m_bShowDebugInfo;
   tGUIPoint m_debugInfoPlacement;
   tGUIColor m_debugInfoTextColor;
   tGUIPoint m_lastMousePos;
#endif
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUICONTEXT_H
