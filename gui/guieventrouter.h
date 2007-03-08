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
/// @class cGUIEventRouter
/// @brief Template base class that implements the IGUIEventRouter methods.

template <typename T, typename ITERLISTENERS>
class cGUIEventRouter
{
public:
   cGUIEventRouter();
   ~cGUIEventRouter();

   tResult GetFocus(IGUIElement * * ppElement);
   tResult SetFocus(IGUIElement * pElement);

protected:
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

template <typename T, typename ITERLISTENERS>
inline IGUIElement * cGUIEventRouter<T, ITERLISTENERS>::AccessFocus()
{
   return m_pFocus;
}

///////////////////////////////////////

template <typename T, typename ITERLISTENERS>
inline IGUIElement * cGUIEventRouter<T, ITERLISTENERS>::AccessMouseOver()
{
   return m_pMouseOver;
}

///////////////////////////////////////

template <typename T, typename ITERLISTENERS>
inline IGUIElement * cGUIEventRouter<T, ITERLISTENERS>::AccessDrag()
{
   return m_pDrag;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIEVENTROUTER_H
