///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_INPUT_H
#define INCLUDED_INPUT_H

#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IInputListener);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IInputListener
//

interface IInputListener : IUnknown
{
   virtual bool OnMouseEvent(int x, int y, uint mouseState, double time) = 0;
   virtual bool OnKeyEvent(long key, bool down, double time) = 0;
};

class cDefaultInputListener : public IInputListener
{
   virtual bool OnMouseEvent(int x, int y, uint mouseState, double time) { return false; }
   virtual bool OnKeyEvent(long key, bool down, double time) { return false; }
};

///////////////////////////////////////////////////////////////////////////////

bool KeyIsDown(long key);
void KeyEvent(long key, bool down, double time);
void MouseEvent(int x, int y, uint mouseState, double time);
void InputAddListener(IInputListener * pListener);
bool InputRemoveListener(IInputListener * pListener);
void InputInit();
void InputTerm();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_INPUT_H
