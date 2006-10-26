///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENT_H
#define INCLUDED_GUIEVENT_H

#include "gui/guiapi.h"
#include "gui/guitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

tGUIString GUIEventName(tGUIEventCode eventCode);
tGUIEventCode GUIEventCode(const char * pszEventName);
tGUIEventCode GUIEventCode(long keyCode, bool down);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIEvent
//

class cGUIEvent : public cComObject<IMPLEMENTS(IGUIEvent)>
{
   cGUIEvent(const cGUIEvent & other);
   const cGUIEvent & operator =(const cGUIEvent & other);

   cGUIEvent();
   cGUIEvent(tGUIEventCode eventCode, const tScreenPoint & mousePos, long keyCode, IGUIElement * pSource,
      bool bCancellable, bool bCtrlKeyDown, bool bAltKeyDown, bool bShiftKeyDown);

public:
   friend tResult GUIEventCreate(tGUIEventCode eventCode, 
                                 tScreenPoint mousePos, 
                                 long keyCode, 
                                 IGUIElement * pSource, 
                                 bool bCancellable, 
                                 bool bCtrlKeyDown,
                                 bool bAltKeyDown,
                                 bool bShiftKeyDown,
                                 IGUIEvent * * ppEvent);

   virtual tResult GetEventCode(tGUIEventCode * pEventCode);
   virtual tResult GetMousePosition(tScreenPoint * pMousePos);
   virtual tResult GetKeyCode(long * pKeyCode);
   virtual tResult GetSourceElement(IGUIElement * * ppElement);

   virtual bool IsCancellable() const;

   virtual tResult GetCancelBubble();
   virtual tResult SetCancelBubble(bool bCancel);

   virtual bool IsCtrlKeyDown() const;
   virtual bool IsAltKeyDown() const;
   virtual bool IsShiftKeyDown() const;

private:
   tGUIEventCode m_eventCode;
   tScreenPoint m_mousePos;
   long m_keyCode;
   cAutoIPtr<IGUIElement> m_pSource;
   bool m_bCancellable;
   bool m_bCancelBubble;
   bool m_bCtrlKeyDown;
   bool m_bAltKeyDown;
   bool m_bShiftKeyDown;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIEVENT_H
