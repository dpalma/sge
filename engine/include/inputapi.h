///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_INPUTAPI_H
#define INCLUDED_INPUTAPI_H

#include "enginedll.h"
#include "comtools.h"
#include "connpt.h"
#include "vec2.h"

#ifdef _MSC_VER
#pragma once
#endif

template <typename T> class cVec2;
typedef class cVec2<float> tVec2;

F_DECLARE_INTERFACE(IInput);
F_DECLARE_INTERFACE(IInputListener);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IInput
//

interface IInput : IUnknown
{
   DECLARE_CONNECTION_POINT(IInputListener);

   /// @remarks The GUI input listener gets first crack at input messages
   /// because the in-game user interface overlays the game
   virtual void SetGUIInputListener(IInputListener * pListener) = 0;

   virtual bool KeyIsDown(long key) = 0;

   virtual void KeyBind(long key, const char * pszDownCmd, const char * pszUpCmd) = 0;
   virtual void KeyUnbind(long key) = 0;

   virtual void ReportKeyEvent(long key, bool down, double time) = 0;
   virtual void ReportMouseEvent(int x, int y, uint mouseState, double time) = 0;
};

ENGINE_API void InputCreate();

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IInputListener
//

struct sInputEvent
{
   long key;
   bool down;
   tVec2 point;
   double time;
};

interface IInputListener : IUnknown
{
   virtual bool OnInputEvent(const sInputEvent * pEvent) = 0;
};

class cDefaultInputListener : public IInputListener
{
   virtual bool OnInputEvent(const sInputEvent * pEvent) { return false; }
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_INPUTAPI_H
