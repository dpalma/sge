///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENTROUTER_H
#define INCLUDED_GUIEVENTROUTER_H

#include "guiapi.h"

#include "connptimpl.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

struct sInputEvent;

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
public:
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

protected:
   ////////////////////////////////////

   tResult GetMouseOver(IGUIElement * * ppElement);
   tResult SetMouseOver(IGUIElement * pElement);

   IGUIElement * AccessFocus();
   IGUIElement * AccessCapture();
   IGUIElement * AccessMouseOver();

   tResult AddElement(IGUIElement * pElement);
   tResult RemoveElement(IGUIElement * pElement);
   tResult GetElements(IGUIElementEnum * * ppElements);
   tResult HasElement(IGUIElement * pElement) const;
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
   tResult GetHitChild(const tGUIPoint & point, IGUIContainerElement * pContainer, IGUIElement * * ppElement) const;

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

#endif // !INCLUDED_GUIEVENTROUTER_H
