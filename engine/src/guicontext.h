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
// TEMPLATE: cGUIEventRouter
//
// Template base class that implements the IGUIEventRouter methods. Mainly
// for testability. A derived class that includes the event routing parts but
// excludes the connection to the real input system, etc. will be used by unit 
// tests.
//
// The template parameter INTRFC is a derivative of IGUIEventRouter
// (e.g., IGUIContext, which inherits from IGUIEventRouter)

template <typename INTRFC>
class cGUIEventRouter : public cConnectionPoint<INTRFC, IGUIEventListener>
{
protected:
   cGUIEventRouter();
   ~cGUIEventRouter();

   ////////////////////////////////////
   // IGUIEventRouter methods

   virtual tResult AddEventListener(IGUIEventListener * pListener);
   virtual tResult RemoveEventListener(IGUIEventListener * pListener);

   virtual tResult GetFocus(IGUIElement * * ppElement);
   virtual tResult SetFocus(IGUIElement * pElement);

   virtual tResult GetCapture(IGUIElement * * ppElement);
   virtual tResult SetCapture(IGUIElement * pElement);

   ////////////////////////////////////

   tResult GetMouseOver(IGUIElement * * ppElement);
   tResult SetMouseOver(IGUIElement * pElement);

   IGUIElement * AccessFocus();
   IGUIElement * AccessCapture();
   IGUIElement * AccessMouseOver();

   tResult AddElement(IGUIElement * pElement);
   void RemoveAllElements();

   template <typename F>
   void ForEachElement(F f)
   {
      tGUIElementList::iterator iter;
      for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
      {
         f(*iter);
      }
   }

   tResult GetHitElement(const tGUIPoint & point, IGUIElement * * ppElement) const;

   bool BubbleEvent(IGUIEvent * pEvent);
   bool GetEventTarget(const sInputEvent * pEvent, IGUIElement * * ppElement);
   bool HandleInputEvent(const sInputEvent * pEvent);

private:
   typedef std::list<IGUIElement *> tGUIElementList;
   tGUIElementList m_elements;

   cAutoIPtr<IGUIElement> m_pFocus, m_pCapture, m_pMouseOver;
};

///////////////////////////////////////

template <typename INTRFC>
inline IGUIElement * cGUIEventRouter<INTRFC>::AccessFocus()
{
   return m_pFocus;
}

///////////////////////////////////////

template <typename INTRFC>
inline IGUIElement * cGUIEventRouter<INTRFC>::AccessCapture()
{
   return m_pCapture;
}

///////////////////////////////////////

template <typename INTRFC>
inline IGUIElement * cGUIEventRouter<INTRFC>::AccessMouseOver()
{
   return m_pMouseOver;
}


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

private:
   class cInputListener : public cComObject<IMPLEMENTS(IInputListener)>
   {
      virtual bool OnInputEvent(const sInputEvent * pEvent);
   };

   friend class cInputListener;
   cInputListener m_inputListener;

   bool m_bNeedLayout;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUICONTEXT_H
