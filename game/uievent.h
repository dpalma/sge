///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIEVENT_H
#define INCLUDED_UIEVENT_H

#include "uitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

class cUIEvent;
class cUIComponent;

///////////////////////////////////////////////////////////////////////////////
//
// ENUM: eUIEventCode
//

enum eUIEventCode
{
   kEventERROR = -1,
   kEventFocus,
   kEventBlur,
   kEventDestroy,
   kEventMouseMove,
   kEventMouseEnter,
   kEventMouseLeave,
   kEventMouseUp,
   kEventMouseDown,
   kEventMouseWheelUp,
   kEventMouseWheelDown,
   kEventKeyUp,
   kEventKeyDown,
   kEventClick,
};

cUIString UIEventName(eUIEventCode code);
eUIEventCode UIEventCode(const char * name);
eUIEventCode UIEventCode(long key, bool down);

///////////////////////////////////////////////////////////////////////////////

bool UIDispatchEvent(cUIComponent * pTarget, const cUIEvent * pEvent, tUIResult * pResult);
bool UIBubbleEvent(cUIComponent * pStart, const cUIEvent * pEvent, tUIResult * pResult);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIEvent
//

class cUIEvent
{
public:
   cUIEvent();
   cUIEvent(const cUIEvent & other);

   const cUIEvent & operator =(const cUIEvent & other);

   eUIEventCode code;
   cUIComponent * pSrc;
   cUIPoint mousePos; // position of mouse in screen coordinates
   long keyCode;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_UIEVENT_H
