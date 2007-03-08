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

template <typename T, typename INTRFC, typename ITERLISTENERS>
cGUIEventRouter<T, INTRFC, ITERLISTENERS>::cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
cGUIEventRouter<T, INTRFC, ITERLISTENERS>::~cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
tResult cGUIEventRouter<T, INTRFC, ITERLISTENERS>::GetFocus(IGUIElement * * ppElement)
{
   Assert(!m_pFocus || m_pFocus->HasFocus());
   return m_pFocus.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
tResult cGUIEventRouter<T, INTRFC, ITERLISTENERS>::SetFocus(IGUIElement * pElement)
{
   if (!!m_pFocus)
   {
      m_pFocus->SetFocus(false);
   }
   SafeRelease(m_pFocus);
   m_pFocus = CTAddRef(pElement);
   if (pElement != NULL)
   {
      pElement->SetFocus(true);
   }
   return S_OK;
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
tResult cGUIEventRouter<T, INTRFC, ITERLISTENERS>::GetMouseOver(IGUIElement * * ppElement)
{
   return m_pMouseOver.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
tResult cGUIEventRouter<T, INTRFC, ITERLISTENERS>::SetMouseOver(IGUIElement * pElement)
{
   SafeRelease(m_pMouseOver);
   m_pMouseOver = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
tResult cGUIEventRouter<T, INTRFC, ITERLISTENERS>::GetDrag(IGUIElement * * ppElement)
{
   return m_pDrag.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
tResult cGUIEventRouter<T, INTRFC, ITERLISTENERS>::SetDrag(IGUIElement * pElement)
{
   SafeRelease(m_pDrag);
   m_pDrag = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
void cGUIEventRouter<T, INTRFC, ITERLISTENERS>::ElementRemoved(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      // If the focus, moused-over, or drag elements are descendants of 
      // the element being removed, release the pointers.
      if (!!m_pFocus && IsDescendant(pElement, m_pFocus))
      {
         SafeRelease(m_pFocus);
      }
      if (!!m_pMouseOver && IsDescendant(pElement, m_pMouseOver))
      {
         SafeRelease(m_pMouseOver);
      }
      if (!!m_pDrag && IsDescendant(pElement, m_pDrag))
      {
         SafeRelease(m_pDrag);
      }
   }
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
bool cGUIEventRouter<T, INTRFC, ITERLISTENERS>::HandleInputEvent(const sInputEvent * pInputEvent)
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

   T * pT = static_cast<T*>(this);

   bool bEatInputEvent = false;

   if (KeyIsMouse(pInputEvent->key))
   {
      cAutoIPtr<IGUIElement> pMouseOver;
      if (pT->GetHitElement(pInputEvent->point, &pMouseOver) != S_OK)
      {
         Assert(!pMouseOver);
      }

      if (eventCode == kGUIEventMouseMove)
      {
         DoMouseEnterExit(pInputEvent, pMouseOver, NULL);
      }

      if (!!pMouseOver
         && pMouseOver->IsEnabled()
         && BubbleEvent(eventCode, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys, pMouseOver, true))
      {
         return true;
      }

      cAutoIPtr<IGUIElement> pDrag;
      if (GetDrag(&pDrag) != S_OK)
      {
         Assert(!pDrag);
      }

      if (!!pMouseOver && pMouseOver->IsEnabled())
      {
         if (eventCode == kGUIEventMouseDown)
         {
            SetFocus(pMouseOver);
            SetDrag(pMouseOver);
         }
         else if (eventCode == kGUIEventMouseUp)
         {
            SetDrag(NULL);

            // If moused-over same as dragging element
            if (CTIsSameObject(pMouseOver, pDrag))
            {
               // Send click to moused-over/dragging element
               BubbleEvent(kGUIEventClick, pInputEvent->point, pInputEvent->key,
                  pInputEvent->modifierKeys, pMouseOver, true);
               bEatInputEvent = true;
            }
         }
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

   return bEatInputEvent;
}

///////////////////////////////////////
// Similar to BubbleEvent but doesn't walk up the parent chain

template <typename T, typename INTRFC, typename ITERLISTENERS>
bool cGUIEventRouter<T, INTRFC, ITERLISTENERS>::DoEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   pEvent->SetCancelBubble(true);
   return BubbleEvent(pEvent);
}

///////////////////////////////////////

template <typename T, typename INTRFC, typename ITERLISTENERS>
bool cGUIEventRouter<T, INTRFC, ITERLISTENERS>::BubbleEvent(IGUIEvent * pEvent)
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

template <typename T, typename INTRFC, typename ITERLISTENERS>
bool cGUIEventRouter<T, INTRFC, ITERLISTENERS>::BubbleEvent(IGUIElement * pStartElement, IGUIEvent * pEvent)
{
   Assert(pStartElement != NULL);
   Assert(pEvent != NULL);

   T * pT = static_cast<T*>(this);

   ITERLISTENERS iter = pT->BeginEventListeners();
   ITERLISTENERS end = pT->EndEventListeners();
   for (; iter != end; ++iter)
   {
      if ((*iter)->OnEvent(pEvent) != S_OK)
      {
         return true;
      }
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

template <typename T, typename INTRFC, typename ITERLISTENERS>
bool cGUIEventRouter<T, INTRFC, ITERLISTENERS>::BubbleEvent(tGUIEventCode eventCode,
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

template <typename T, typename INTRFC, typename ITERLISTENERS>
void cGUIEventRouter<T, INTRFC, ITERLISTENERS>::DoMouseEnterExit(const sInputEvent * pInputEvent,
                                                                 IGUIElement * pMouseOver,
                                                                 IGUIElement * pRestrictTo)
{
   if (pMouseOver != NULL)
   {
      bool bMouseOverSame = false;

      cAutoIPtr<IGUIElement> pOldMouseOver;
      if (GetMouseOver(&pOldMouseOver) == S_OK)
      {
         bMouseOverSame = CTIsSameObject(pMouseOver, pOldMouseOver);

         if (!bMouseOverSame)
         {
            SetMouseOver(NULL);
            pOldMouseOver->SetMouseOver(false);

            cAutoIPtr<IGUIEvent> pMouseLeaveEvent;
            if (GUIEventCreate(kGUIEventMouseLeave, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys,
               pOldMouseOver, true, &pMouseLeaveEvent) == S_OK)
            {
               DoEvent(pMouseLeaveEvent);
            }
         }
      }

      if (!bMouseOverSame 
         && ((pRestrictTo == NULL) || (CTIsSameObject(pMouseOver, pRestrictTo))))
      {
         SetMouseOver(pMouseOver);
         pMouseOver->SetMouseOver(true);

         cAutoIPtr<IGUIEvent> pMouseEnterEvent;
         if (GUIEventCreate(kGUIEventMouseEnter, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys,
            pMouseOver, true, &pMouseEnterEvent) == S_OK)
         {
            DoEvent(pMouseEnterEvent);
         }
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
         pOldMouseOver->SetMouseOver(false);

         cAutoIPtr<IGUIEvent> pMouseLeaveEvent;
         if (GUIEventCreate(kGUIEventMouseLeave, pInputEvent->point, pInputEvent->key, pInputEvent->modifierKeys,
            pOldMouseOver, true, &pMouseLeaveEvent) == S_OK)
         {
            DoEvent(pMouseLeaveEvent);
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////

#include "tech/undbgalloc.h"

#endif // !INCLUDED_GUIEVENTROUTERTEM_H
