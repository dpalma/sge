///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENTROUTER_H
#define INCLUDED_GUIEVENTROUTER_H

#include "gui/guiapi.h"

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

template <typename T, typename INTRFC, typename ITERLISTENERS>
class cGUIEventRouter : public INTRFC
{
public:
   cGUIEventRouter();
   ~cGUIEventRouter();

   ////////////////////////////////////
   // IGUIEventRouter methods

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

   bool HandleInputEvent(const sInputEvent * pInputEvent);

private:
   bool DoEvent(IGUIEvent * pEvent);
   bool BubbleEvent(IGUIEvent * pEvent);
   bool BubbleEvent(IGUIElement * pStartElement, IGUIEvent * pEvent);
   bool BubbleEvent(tGUIEventCode eventCode, tScreenPoint mousePos, long keyCode,
      int modifierKeys, IGUIElement * pSource, bool bCancellable);

   void DoMouseEnterExit(const sInputEvent * pInputEvent, IGUIElement * pMouseOver, IGUIElement * pRestrictTo);

   cAutoIPtr<IGUIElement> m_pFocus;
   cAutoIPtr<IGUIElement> m_pMouseOver, m_pDrag;
};

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
inline IGUIElement * cGUIEventRouter<T, INTRFC, ITERLISTENERS>::AccessFocus()
{
   return m_pFocus;
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
inline IGUIElement * cGUIEventRouter<T, INTRFC, ITERLISTENERS>::AccessMouseOver()
{
   return m_pMouseOver;
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
inline IGUIElement * cGUIEventRouter<T, INTRFC, ITERLISTENERS>::AccessDrag()
{
   return m_pDrag;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIEVENTROUTER_H
