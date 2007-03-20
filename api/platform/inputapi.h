///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_INPUTAPI_H
#define INCLUDED_INPUTAPI_H

#include "platformdll.h"
#include "tech/comtools.h"
#include "tech/vec2.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IInput);
F_DECLARE_INTERFACE(IInputListener);
F_DECLARE_INTERFACE(IInputModalListener);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IInput
//

interface IInput : IUnknown
{
   virtual tResult AddInputListener(IInputListener * pListener) = 0;
   virtual tResult RemoveInputListener(IInputListener * pListener) = 0;

   virtual bool KeyIsDown(long key) = 0;

   virtual void KeyBind(long key, const char * pszDownCmd, const char * pszUpCmd) = 0;
   virtual void KeyUnbind(long key) = 0;

   virtual tResult PushModalListener(IInputModalListener * pModalListener) = 0;
   virtual tResult PopModalListener() = 0;
};

////////////////////////////////////////

PLATFORM_API tResult InputCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IInputListener
//

struct sInputEvent
{
   long key;
   int modifierKeys;
   bool down;
   cVec2<int> point;
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
//
// INTERFACE: IInputModalListener
//

interface IInputModalListener : IInputListener
{
   virtual void CancelMode() = 0;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_INPUTAPI_H
