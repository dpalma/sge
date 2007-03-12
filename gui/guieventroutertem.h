///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENTROUTERTEM_H
#define INCLUDED_GUIEVENTROUTERTEM_H

/// @file guieventroutertem.h
/// Contains the member function implementations of the cGUIEventRouter class.
/// This file is intended to be included only by cpp files, not other headers.

#include "gui/guiapi.h"

#include "guielementtools.h"
#include "guievent.h"

#include "platform/inputapi.h"

#include <algorithm>

#include "tech/dbgalloc.h" // must be last header

#ifdef _MSC_VER
#pragma once
#endif

LOG_EXTERN_CHANNEL(GUIEventRouter); // defined in guieventrouter.cpp

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIEventRouter
//

///////////////////////////////////////

template <typename T>
cGUIEventRouter<T>::cGUIEventRouter()
 : m_pMouseHandler(&cGUIEventRouter<T>::HandleMouseEventSteadyState)
{
}

///////////////////////////////////////

template <typename T>
cGUIEventRouter<T>::~cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename T>
bool cGUIEventRouter<T>::HandleInputEvent(const sInputEvent * pInputEvent)
{
   DebugMsgIfEx5(GUIEventRouter, pInputEvent->key != kMouseMove,
      "InputEvent: key %d, down %d, point(%d, %d), time %f\n",
      pInputEvent->key, static_cast<int>(pInputEvent->down),
      pInputEvent->point.x, pInputEvent->point.y,
      pInputEvent->time);

   // The input system generates two events for mousewheel actions--one with the key-down
   // flag set true, and one with it set false; Ignore the false one so that only one GUI
   // mousewheel event occurs
   if ((pInputEvent->key == kMouseWheelUp || pInputEvent->key == kMouseWheelDown) && !pInputEvent->down)
   {
      return false;
   }

   tGUIEventCode eventCode = GUIEventCode(pInputEvent->key, pInputEvent->down);
   if (eventCode == kGUIEventNone)
   {
      WarnMsg2("Unable to determine GUI event code for input event arguments %d, %d\n",
         pInputEvent->key, pInputEvent->down);
      return false;
   }

   if (KeyIsMouse(pInputEvent->key))
   {
      T * pT = static_cast<T*>(this);

      cAutoIPtr<IGUIElement> pNewMouseOver;
      if (pT->GetHitElement(pInputEvent->point, &pNewMouseOver) != S_OK)
      {
         Assert(!pNewMouseOver);
      }

      if (m_pMouseHandler != NULL)
      {
         return (this->*m_pMouseHandler)(pInputEvent, eventCode, pNewMouseOver);
      }
   }
   else
   {
      cAutoIPtr<IGUIElement> pFocus;
      if (GetFocus(&pFocus) == S_OK)
      {
         Assert(!!pFocus);
         return BubbleEvent(eventCode, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys, pFocus, true);
      }
   }

   return false;
}

///////////////////////////////////////

template <typename T>
tResult cGUIEventRouter<T>::GetFocus(IGUIElement * * ppElement)
{
   return m_pFocus.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename T>
tResult cGUIEventRouter<T>::SetFocus(IGUIElement * pElement)
{
   SafeRelease(m_pFocus);
   m_pFocus = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename T>
tResult cGUIEventRouter<T>::GetMouseOver(IGUIElement * * ppElement)
{
   return m_pMouseOver.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename T>
tResult cGUIEventRouter<T>::SetMouseOver(IGUIElement * pElement)
{
   SafeRelease(m_pMouseOver);
   m_pMouseOver = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename T>
tResult cGUIEventRouter<T>::GetArmed(IGUIElement * * ppElement)
{
   return m_pArmed.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename T>
tResult cGUIEventRouter<T>::SetArmed(IGUIElement * pElement, const tScreenPoint & armedAtPoint)
{
   SafeRelease(m_pArmed);
   m_pArmed = CTAddRef(pElement);
   m_armedAtPoint = armedAtPoint;
   return S_OK;
}

///////////////////////////////////////

template <typename T>
void cGUIEventRouter<T>::ElementRemoved(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      // If the focus, moused-over, or armed elements are descendants of 
      // the element being removed, release the pointers.
      if (!!m_pFocus && IsDescendant(pElement, m_pFocus))
      {
         SafeRelease(m_pFocus);
      }
      if (!!m_pMouseOver && IsDescendant(pElement, m_pMouseOver))
      {
         SafeRelease(m_pMouseOver);
      }
      if (!!m_pArmed && IsDescendant(pElement, m_pArmed))
      {
         SafeRelease(m_pArmed);
      }
   }
}

///////////////////////////////////////
// Similar to BubbleEvent but doesn't walk up the parent chain

template <typename T>
bool cGUIEventRouter<T>::DoEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   pEvent->SetCancelBubble(true);
   return BubbleEvent(pEvent);
}

///////////////////////////////////////

template <typename T>
bool cGUIEventRouter<T>::BubbleEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   cAutoIPtr<IGUIElement> pEventSrc;
   if (pEvent->GetSourceElement(&pEventSrc) == S_OK)
   {
      return BubbleEvent(pEventSrc, pEvent);
   }

   return false;
}

///////////////////////////////////////

template <typename T>
bool cGUIEventRouter<T>::BubbleEvent(IGUIElement * pStartElement, IGUIEvent * pEvent)
{
   Assert(pStartElement != NULL);
   Assert(pEvent != NULL);

   T * pT = static_cast<T*>(this);

   if (pT->NotifyListeners(pEvent))
   {
      return true;
   }

   cAutoIPtr<IGUIElement> pDispatchTo(CTAddRef(pStartElement));
   while (!!pDispatchTo)
   {
      if (pDispatchTo->OnEvent(pEvent) != S_OK)
      {
         return true;
      }

      if (pEvent->GetCancelBubble() == S_OK)
      {
         break;
      }

      cAutoIPtr<IGUIElement> pNext;
      if (pDispatchTo->GetParent(&pNext) != S_OK)
      {
         SafeRelease(pDispatchTo);
      }
      else
      {
         pDispatchTo = pNext;
      }
   }

   return false;
}

///////////////////////////////////////

template <typename T>
bool cGUIEventRouter<T>::BubbleEvent(tGUIEventCode eventCode,
                                     tScreenPoint mousePos,
                                     long keyCode,
                                     int modifierKeys,
                                     IGUIElement * pSource,
                                     bool bCancellable)
{
   cAutoIPtr<IGUIEvent> pEvent;
   if (GUIEventCreate(eventCode, mousePos,  keyCode, modifierKeys, pSource, bCancellable, &pEvent) == S_OK)
   {
      return BubbleEvent(pEvent);
   }
   else
   {
      return false;
   }
}

///////////////////////////////////////

template <typename T>
tResult cGUIEventRouter<T>::DoMouseEnterLeave(const sInputEvent * pInputEvent,
                                              IGUIElement * pNewMouseOver,
                                              IGUIElement * pRestrictTo,
                                              tGUIEventCode * pEventCode,
                                              IGUIElement * * ppSourceElement)
{
   Assert(pEventCode != NULL);
   Assert(ppSourceElement != NULL);

   if (pNewMouseOver != NULL)
   {
      bool bMouseOverSame = false;

      cAutoIPtr<IGUIElement> pOldMouseOver;
      if (GetMouseOver(&pOldMouseOver) == S_OK)
      {
         bMouseOverSame = CTIsSameObject(pNewMouseOver, pOldMouseOver);

         if (!bMouseOverSame)
         {
            SetMouseOver(NULL);

            *pEventCode = kGUIEventMouseLeave;
            *ppSourceElement = CTAddRef(pOldMouseOver);
            return S_OK;
         }
      }

      if (!bMouseOverSame 
         && ((pRestrictTo == NULL) || (CTIsSameObject(pNewMouseOver, pRestrictTo))))
      {
         SetMouseOver(pNewMouseOver);

         *pEventCode = kGUIEventMouseEnter;
         *ppSourceElement = CTAddRef(pNewMouseOver);
         return S_OK;
      }
   }
   else
   {
      cAutoIPtr<IGUIElement> pOldMouseOver;
      if (GetMouseOver(&pOldMouseOver) == S_OK)
      {
         tGUIPoint absPos = GUIElementAbsolutePosition(pOldMouseOver);

         tGUIPoint relPoint(pInputEvent->point.x - absPos.x, pInputEvent->point.y - absPos.y); // TODO: ADDED_tScreenPoint

         SetMouseOver(NULL);

         *pEventCode = kGUIEventMouseLeave;
         *ppSourceElement = CTAddRef(pOldMouseOver);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

template <typename T>
bool cGUIEventRouter<T>::HandleMouseEventSteadyState(const sInputEvent * pInputEvent,
                                                     tGUIEventCode eventCode,
                                                     IGUIElement * pNewMouseOver)
{
   if (eventCode == kGUIEventMouseMove)
   {
      tGUIEventCode enterLeaveEventCode = kGUIEventNone;
      cAutoIPtr<IGUIElement> pEnterLeaveElement;
      if (DoMouseEnterLeave(pInputEvent, pNewMouseOver, NULL, &enterLeaveEventCode, &pEnterLeaveElement) == S_OK)
      {
         cAutoIPtr<IGUIEvent> pEvent;
         if (GUIEventCreate(enterLeaveEventCode, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys,
            pEnterLeaveElement, true, &pEvent) == S_OK)
         {
            DoEvent(pEvent);
         }
      }
   }

   if ((pNewMouseOver != NULL)
      && pNewMouseOver->IsEnabled()
      && BubbleEvent(eventCode, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys, pNewMouseOver, true))
   {
      return true;
   }

   if ((eventCode == kGUIEventMouseDown) && (pNewMouseOver != NULL) && pNewMouseOver->IsEnabled())
   {
      SetFocus(pNewMouseOver);
      SetArmed(pNewMouseOver, pInputEvent->point);
      m_pMouseHandler = &cGUIEventRouter<T>::HandleMouseEventClicking;
      return true;
   }

   return false;
}

///////////////////////////////////////

template <typename T>
bool cGUIEventRouter<T>::HandleMouseEventClicking(const sInputEvent * pInputEvent,
                                                  tGUIEventCode eventCode,
                                                  IGUIElement * pNewMouseOver)
{
   if (eventCode == kGUIEventMouseMove)
   {
      tGUIEventCode enterLeaveEventCode = kGUIEventNone;
      cAutoIPtr<IGUIElement> pEnterLeaveElement;
      if (DoMouseEnterLeave(pInputEvent, pNewMouseOver, NULL, &enterLeaveEventCode, &pEnterLeaveElement) == S_OK)
      {
         cAutoIPtr<IGUIEvent> pEvent;
         if (GUIEventCreate(enterLeaveEventCode, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys,
            pEnterLeaveElement, true, &pEvent) == S_OK)
         {
            DoEvent(pEvent);
         }
      }

      // Test for the start of a drag
      cAutoIPtr<IGUIElement> pArmed;
      if (GetArmed(&pArmed) == S_OK)
      {
         int distSqr = Vec2DistanceSqr(pInputEvent->point, m_armedAtPoint);
         if ((enterLeaveEventCode == kGUIEventMouseLeave) || (distSqr >= kDragThresholdSqr))
         {
            Assert(!m_pDragSource);
            if (pArmed->QueryInterface(IID_IGUIDragSource, (void**)&m_pDragSource) == S_OK)
            {
               m_pMouseHandler = &cGUIEventRouter<T>::HandleMouseEventDragging;
               return true;
            }
         }
      }
   }

   if ((pNewMouseOver != NULL)
      && pNewMouseOver->IsEnabled()
      && BubbleEvent(eventCode, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys, pNewMouseOver, true))
   {
      return true;
   }

   if (eventCode == kGUIEventMouseUp)
   {
      m_pMouseHandler = &cGUIEventRouter<T>::HandleMouseEventSteadyState;

      cAutoIPtr<IGUIElement> pArmed;
      if (GetArmed(&pArmed) == S_OK)
      {
         SetArmed(NULL, tScreenPoint(0,0));

         // If the moused-over and the armed element are the same
         // on mouse-up, generate a click event
         if (CTIsSameObject(pNewMouseOver, pArmed))
         {
            BubbleEvent(kGUIEventClick, pInputEvent->point, pInputEvent->key,
               pInputEvent->modifierKeys, pArmed, true);
            return true;
         }
      }
   }

   return false;
}

///////////////////////////////////////

template <typename T>
bool cGUIEventRouter<T>::HandleMouseEventDragging(const sInputEvent * pInputEvent,
                                                  tGUIEventCode eventCode,
                                                  IGUIElement * pNewMouseOver)
{
   Assert(!!m_pDragSource);

   if (eventCode == kGUIEventMouseMove)
   {
      tGUIEventCode enterLeaveEventCode = kGUIEventNone;
      cAutoIPtr<IGUIElement> pEnterLeaveElement;
      if (DoMouseEnterLeave(pInputEvent, pNewMouseOver, NULL, &enterLeaveEventCode, &pEnterLeaveElement) == S_OK)
      {
         cAutoIPtr<IGUIEvent> pEvent;
         if (GUIEventCreate(enterLeaveEventCode, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys,
            pEnterLeaveElement, true, &pEvent) == S_OK)
         {
            //DoEvent(pEvent);
         }
      }
   }
   else if (eventCode == kGUIEventMouseUp)
   {
      EndDrag();
   }

   return false;
}

///////////////////////////////////////

template <typename T>
void cGUIEventRouter<T>::EndDrag()
{
   m_pMouseHandler = &cGUIEventRouter<T>::HandleMouseEventSteadyState;
   SafeRelease(m_pDragSource);
}


///////////////////////////////////////////////////////////////////////////////

#include "tech/undbgalloc.h"

#endif // !INCLUDED_GUIEVENTROUTERTEM_H
