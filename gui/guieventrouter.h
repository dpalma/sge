///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENTROUTER_H
#define INCLUDED_GUIEVENTROUTER_H

#include "gui/guiapi.h"

#include "tech/connptimpl.h"

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
//
/// @class cGUIEventRouter
/// @brief Template base class that implements the IGUIEventRouter methods.

template <typename T, typename INTRFC>
class cGUIEventRouter : public cConnectionPoint<INTRFC, IGUIEventListener>
{
   typedef cConnectionPoint<INTRFC, IGUIEventListener> tBaseClass;

public:
   cGUIEventRouter();
   ~cGUIEventRouter();

   ////////////////////////////////////
   // IGUIEventRouter methods

   virtual tResult AddEventListener(IGUIEventListener * pListener);
   virtual tResult RemoveEventListener(IGUIEventListener * pListener);

   virtual tResult GetFocus(IGUIElement * * ppElement);
   virtual tResult SetFocus(IGUIElement * pElement);

protected:
   ////////////////////////////////////

   tResult GetMouseOver(IGUIElement * * ppElement);
   tResult SetMouseOver(IGUIElement * pElement);

   tResult GetDrag(IGUIElement * * ppElement);
   tResult SetDrag(IGUIElement * pElement);

   IGUIElement * AccessFocus();
   IGUIElement * AccessMouseOver();
   IGUIElement * AccessDrag();

   void ElementRemoved(IGUIElement * pElement);

   bool DoEvent(IGUIEvent * pEvent);
   bool BubbleEvent(IGUIEvent * pEvent);
   bool BubbleEvent(IGUIElement * pStartElement, IGUIEvent * pEvent);

   void DoMouseEnterExit(const sInputEvent * pInputEvent, IGUIElement * pMouseOver, IGUIElement * pRestrictTo);

   bool HandleInputEvent(const sInputEvent * pInputEvent);

private:
   cAutoIPtr<IGUIElement> m_pFocus, m_pMouseOver, m_pDrag;
};

///////////////////////////////////////

template <typename T, typename INTRFC>
inline IGUIElement * cGUIEventRouter<T, INTRFC>::AccessFocus()
{
   return m_pFocus;
}

///////////////////////////////////////

template <typename T, typename INTRFC>
inline IGUIElement * cGUIEventRouter<T, INTRFC>::AccessMouseOver()
{
   return m_pMouseOver;
}

///////////////////////////////////////

template <typename T, typename INTRFC>
inline IGUIElement * cGUIEventRouter<T, INTRFC>::AccessDrag()
{
   return m_pDrag;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIEVENTROUTER_H
