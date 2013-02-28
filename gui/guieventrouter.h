///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENTROUTER_H
#define INCLUDED_GUIEVENTROUTER_H

#include "gui/guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

struct sInputEvent;

class cGUINotifyListeners
{
public:
   virtual bool NotifyListeners(IGUIEvent *) = 0;
};

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

template <typename T>
class cGUIEventRouter
{
public:
   cGUIEventRouter();
   ~cGUIEventRouter();

   bool HandleInputEvent(const sInputEvent * pInputEvent);

   tResult GetFocus(IGUIElement * * ppElement);
   tResult SetFocus(IGUIElement * pElement);

protected:
   tResult GetMouseOver(IGUIElement * * ppElement);
   tResult SetMouseOver(IGUIElement * pElement);

   tResult GetArmed(IGUIElement * * ppElement);
   tResult SetArmed(IGUIElement * pElement, const tScreenPoint & armedAtPoint);

   IGUIElement * AccessFocus();
   IGUIElement * AccessMouseOver();
   IGUIElement * AccessArmed();

   void ElementRemoved(IGUIElement * pElement);

private:
   bool DoEvent(IGUIEvent * pEvent);
   bool BubbleEvent(IGUIEvent * pEvent);
   bool BubbleEvent(IGUIElement * pStartElement, IGUIEvent * pEvent);
   bool BubbleEvent(tGUIEventCode eventCode, tScreenPoint mousePos, long keyCode,
      int modifierKeys, IGUIElement * pSource, bool bCancellable);

   tResult DoMouseEnterLeave(const sInputEvent * pInputEvent, IGUIElement * pNewMouseOver, IGUIElement * pRestrictTo,
      tGUIEventCode * pEventCode, IGUIElement * * ppSourceElement);

   typedef bool (cGUIEventRouter<T>::*tInputEventHandlerFn)(const sInputEvent *, tGUIEventCode, IGUIElement *);

   bool HandleMouseEventSteadyState(const sInputEvent * pInputEvent, tGUIEventCode eventCode, IGUIElement * pNewMouseOver);
   bool HandleMouseEventClicking(const sInputEvent * pInputEvent, tGUIEventCode eventCode, IGUIElement * pNewMouseOver);
   bool HandleMouseEventDragging(const sInputEvent * pInputEvent, tGUIEventCode eventCode, IGUIElement * pNewMouseOver);

   void EndDrag();

   enum eConstants
   {
      kDragThreshold = 4,
      kDragThresholdSqr = (kDragThreshold * kDragThreshold),
   };

   cAutoIPtr<IGUIElement> m_pFocus;
   cAutoIPtr<IGUIElement> m_pMouseOver;
   cAutoIPtr<IGUIElement> m_pArmed; // the "armed" element is the one currently being clicked or dragged
   tScreenPoint m_armedAtPoint; // the mouse point at which the click or drag was initiated
   tInputEventHandlerFn m_pMouseHandler;
   cAutoIPtr<IGUIDragSource> m_pDragSource;
};

///////////////////////////////////////

template <typename T>
inline IGUIElement * cGUIEventRouter<T>::AccessFocus()
{
   return m_pFocus;
}

///////////////////////////////////////

template <typename T>
inline IGUIElement * cGUIEventRouter<T>::AccessMouseOver()
{
   return m_pMouseOver;
}

///////////////////////////////////////

template <typename T>
inline IGUIElement * cGUIEventRouter<T>::AccessArmed()
{
   return m_pArmed;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIEVENTROUTER_H
