///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_INPUT_H
#define INCLUDED_INPUT_H

#include "inputapi.h"
#include "globalobj.h"
#include "connptimpl.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cInput
//

class cInput : public cGlobalObject<IMPLEMENTSCP(IInput, IInputListener)>
{
public:
   cInput();

   virtual tResult Init();
   virtual tResult Term();

   virtual bool KeyIsDown(long key);

   virtual void KeyBind(long key, const char * pszDownCmd, const char * pszUpCmd);
   virtual void KeyUnbind(long key);

   virtual void ReportKeyEvent(long key, bool down, double time);
   virtual void ReportMouseEvent(int x, int y, uint mouseState, double time);

private:
   bool DispatchInputEvent(int x, int y, long key, bool down, double time);

   const char * KeyGetDownBinding(long key) const;
   const char * KeyGetUpBinding(long key) const;

   enum { kMaxKeys = 256 };
   ulong m_keyRepeats[kMaxKeys];
   char * m_keyDownBindings[kMaxKeys];
   char * m_keyUpBindings[kMaxKeys];

   uint m_oldMouseState;
};

///////////////////////////////////////

inline const char * cInput::KeyGetDownBinding(long key) const
{
   Assert(key > -1 && key < kMaxKeys);
   if (key > -1 && key < kMaxKeys)
      return m_keyDownBindings[key];
   else
      return NULL;
}

///////////////////////////////////////

inline const char * cInput::KeyGetUpBinding(long key) const
{
   Assert(key > -1 && key < kMaxKeys);
   if (key > -1 && key < kMaxKeys)
      return m_keyUpBindings[key];
   else
      return NULL;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_INPUT_H
