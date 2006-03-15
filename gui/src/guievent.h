///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENT_H
#define INCLUDED_GUIEVENT_H

#include "guiapi.h"
#include "guitypes.h"

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
   cGUIEvent(tGUIEventCode eventCode, const tScreenPoint & mousePos, long keyCode, 
      IGUIElement * pSource, bool bCancellable);

public:
   friend tResult GUIEventCreate(tGUIEventCode eventCode, 
                                 tScreenPoint mousePos, 
                                 long keyCode, 
                                 IGUIElement * pSource, 
                                 bool bCancellable, 
                                 IGUIEvent * * ppEvent);

   virtual tResult GetEventCode(tGUIEventCode * pEventCode);
   virtual tResult GetMousePosition(tScreenPoint * pMousePos);
   virtual tResult GetKeyCode(long * pKeyCode);
   virtual tResult GetSourceElement(IGUIElement * * ppElement);

   virtual bool IsCancellable() const;

   virtual tResult GetCancelBubble();
   virtual tResult SetCancelBubble(bool bCancel);

private:
   tGUIEventCode m_eventCode;
   tScreenPoint m_mousePos;
   long m_keyCode;
   cAutoIPtr<IGUIElement> m_pSource;
   bool m_bCancellable;
   bool m_bCancelBubble;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIEVENT_H
