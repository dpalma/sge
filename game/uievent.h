///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIEVENT_H
#define INCLUDED_UIEVENT_H

#include "uitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

struct sInputEvent;

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

bool UIDispatchEvent(cUIComponent * pTarget, const cUIEvent * pEvent);
bool UIBubbleEvent(cUIComponent * pStart, const cUIEvent * pEvent);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIEvent
//

class cUIEvent
{
public:
   cUIEvent();
   cUIEvent(const sInputEvent * pInputEvent);
   cUIEvent(eUIEventCode code, const cUIPoint & mousePos, long keyCode, cUIComponent * pSrc = NULL);
   cUIEvent(const cUIEvent & other);
   const cUIEvent & operator =(const cUIEvent & other);

   eUIEventCode code;
   cUIPoint mousePos; // position of mouse in screen coordinates
   long keyCode;
   cUIComponent * pSrc;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_UIEVENT_H
