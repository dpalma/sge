///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_INPUTAPI_H
#define INCLUDED_INPUTAPI_H

#include "enginedll.h"
#include "comtools.h"
#include "connpt.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IInput);
F_DECLARE_INTERFACE(IInputListener);

F_DECLARE_INTERFACE(IWindow);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IInput
//

interface IInput : IUnknown
{
   DECLARE_CONNECTION_POINT(IInputListener);

   virtual bool KeyIsDown(long key) = 0;

   virtual void KeyBind(long key, const char * pszDownCmd, const char * pszUpCmd) = 0;
   virtual void KeyUnbind(long key) = 0;

   virtual tResult AddWindow(IWindow * pWindow) = 0;
   virtual tResult RemoveWindow(IWindow * pWindow) = 0;
};

ENGINE_API void InputCreate();

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

#endif // !INCLUDED_INPUTAPI_H
