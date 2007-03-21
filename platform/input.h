///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_INPUT_H
#define INCLUDED_INPUT_H

#include "platform/inputapi.h"
#include "tech/globalobjdef.h"
#include "tech/connptimpl.h"

#include <map>
#include <stack>

#ifdef _MSC_VER
#pragma once
#endif


const int kMaxKeys = 256;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cKeyBindings
//

class cKeyBindings
{
public:
   cKeyBindings();
   ~cKeyBindings();

   tResult Bind(long key, IInputKeyBindTarget * pKeyBindTarget);
   void Unbind(long key);
   void UnbindAll();

   void ExecuteBinding(long key, bool down, double time);

private:
   typedef std::map<long, IInputKeyBindTarget *> tKeyBindTargetMap;
   tKeyBindTargetMap m_keyBindTargetMap;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cInput
//

class cInput : public cComObject2<IMPLEMENTSCP(IInput, IInputListener),
                                  IMPLEMENTS(IGlobalObject)>
             , public cKeyBindings
{
public:
   cInput();
   ~cInput();

   DECLARE_NAME(Input)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult AddInputListener(IInputListener * pListener);
   virtual tResult RemoveInputListener(IInputListener * pListener);

   virtual bool KeyIsDown(long key);

   virtual void KeyBind(long key, const char * pszDownCmd, const char * pszUpCmd);
   virtual tResult KeyBind(long key, IInputKeyBindTarget * pKeyBindTarget);
   virtual void KeyUnbind(long key);

   virtual tResult PushModalListener(IInputModalListener * pModalListener);
   virtual tResult PopModalListener();

protected:
   void ReportKeyEvent(long key, bool down, double time);
   void ReportMouseEvent(int x, int y, uint mouseState, double time);

   static void OnSysCharEvent(tChar c, double time, uint_ptr userData);
   static void OnSysKeyEvent(long key, bool down, double time, uint_ptr userData);
   static void OnSysMouseMove(int x, int y, uint mouseState, double time, uint_ptr userData);
   static void OnSysMouseEvent(int x, int y, uint mouseState, double time, uint_ptr userData);

private:
   bool DispatchInputEvent(int x, int y, long key, bool down, double time);

   class cInputModalListeners : public cComObject<IMPLEMENTS(IInputListener)>
   {
   public:
      cInputModalListeners(cInput * pOuter);
      virtual bool OnInputEvent(const sInputEvent * pEvent);
      virtual tResult PushModalListener(IInputModalListener * pModalListener);
      virtual tResult PopModalListener();
      void CancelAll();
   private:
      cInput * m_pOuter;
      std::stack<IInputModalListener*> m_modalListeners;
   };

   friend class cInputModalListeners;
   cInputModalListeners m_modalListeners;

   ulong m_keyRepeats[kMaxKeys];

   uint m_oldMouseState;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_INPUT_H
