///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTEXT_H
#define INCLUDED_GUICONTEXT_H

#include "guiapi.h"

#include "inputapi.h"

#include "globalobj.h"
#include "connptimpl.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

class cGUIContext : public cGlobalObject<IMPLEMENTSCP(IGUIContext, IGUIEventListener)>
{
public:
   cGUIContext();
   ~cGUIContext();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult AddEventListener(IGUIEventListener * pListener);
   virtual tResult RemoveEventListener(IGUIEventListener * pListener);

   virtual tResult GetFocus(IGUIElement * * ppElement);
   virtual tResult SetFocus(IGUIElement * pElement);

   virtual tResult GetCapture(IGUIElement * * ppElement);
   virtual tResult SetCapture(IGUIElement * pElement);

   virtual tResult LoadFromResource(const char * psz);
   virtual tResult LoadFromString(const char * psz);

   virtual tResult RenderGUI(IRenderDevice * pRenderDevice);

private:
   tResult GetMouseOver(IGUIElement * * ppElement);
   tResult SetMouseOver(IGUIElement * pElement);

   IGUIElement * AccessFocus();
   IGUIElement * AccessCapture();
   IGUIElement * AccessMouseOver();

   bool BubbleEvent(IGUIEvent * pEvent);
   bool GetEventTarget(const sInputEvent * pEvent, IGUIElement * * ppElement);
   bool HandleInputEvent(const sInputEvent * pEvent);

   class cInputListener : public cComObject<IMPLEMENTS(IInputListener)>
   {
      virtual bool OnInputEvent(const sInputEvent * pEvent);
   };

   friend class cInputListener;
   cInputListener m_inputListener;

   typedef std::list<IGUIElement *> tGUIElementList;
   tGUIElementList m_elements;

   cAutoIPtr<IGUIElement> m_pFocus, m_pCapture, m_pMouseOver;
};

///////////////////////////////////////

inline IGUIElement * cGUIContext::AccessFocus()
{
   return m_pFocus;
}

///////////////////////////////////////

inline IGUIElement * cGUIContext::AccessCapture()
{
   return m_pCapture;
}

///////////////////////////////////////

inline IGUIElement * cGUIContext::AccessMouseOver()
{
   return m_pMouseOver;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUICONTEXT_H
