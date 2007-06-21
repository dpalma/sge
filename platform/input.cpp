///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "input.h"

#include "platform/keys.h"
#include "platform/sys.h"
#include "script/scriptapi.h"
#include "tech/point2.inl"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cstring>
#include <cstdlib>
#include <functional>
#include <locale>

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(InputEvents);


///////////////////////////////////////////////////////////////////////////////

inline long KeyGetBindable(long key)
{
   // not allowed to bind uppercase keys
   if (isalpha(key))
   {
      return tolower(key);
   }
   else
   {
      return key;
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScriptKeyBindTarget
//

class cScriptKeyBindTarget : public cComObject<IMPLEMENTS(IInputKeyBindTarget)>
{
public:
   cScriptKeyBindTarget(const char * pszDownCmd, const char * pszUpCmd);

   virtual void ExecuteBinding(long key, bool down, double time);

private:
   cStr m_down, m_up;
};

///////////////////////////////////////

cScriptKeyBindTarget::cScriptKeyBindTarget(const char * pszDownCmd, const char * pszUpCmd)
 : m_down((pszDownCmd != NULL) ? pszDownCmd : "")
 , m_up((pszUpCmd != NULL) ? pszUpCmd : "")
{
}

///////////////////////////////////////

void cScriptKeyBindTarget::ExecuteBinding(long key, bool down, double time)
{
   const cStr & binding = down ? m_down : m_up;
   if (!binding.empty())
   {
      UseGlobal(ScriptInterpreter);
      pScriptInterpreter->ExecString(binding.c_str());
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cKeyBindings
//

///////////////////////////////////////

cKeyBindings::cKeyBindings()
{
}

///////////////////////////////////////

cKeyBindings::~cKeyBindings()
{
   UnbindAll();
}

///////////////////////////////////////

tResult cKeyBindings::Bind(long key, IInputKeyBindTarget * pKeyBindTarget)
{
   Assert(key > -1 && key < kMaxKeys);
   Assert(pKeyBindTarget != NULL);

   key = KeyGetBindable(key);

   tKeyBindTargetMap::iterator f = m_keyBindTargetMap.find(key);
   if (f != m_keyBindTargetMap.end())
   {
      SafeRelease(f->second);
   }

   m_keyBindTargetMap[key] = CTAddRef(pKeyBindTarget);

   return S_OK;
}

///////////////////////////////////////

void cKeyBindings::Unbind(long key)
{
   Assert(key > -1 && key < kMaxKeys);

   key = KeyGetBindable(key);

   tKeyBindTargetMap::iterator f = m_keyBindTargetMap.find(key);
   if (f != m_keyBindTargetMap.end())
   {
      SafeRelease(f->second);
      m_keyBindTargetMap.erase(f);
   }
}

///////////////////////////////////////

void cKeyBindings::UnbindAll()
{
   tKeyBindTargetMap::iterator iter = m_keyBindTargetMap.begin(), end = m_keyBindTargetMap.end();
   for (; iter != end; ++iter)
   {
      SafeRelease(iter->second);
   }
   m_keyBindTargetMap.clear();
}

///////////////////////////////////////

void cKeyBindings::ExecuteBinding(long key, bool down, double time)
{
   tKeyBindTargetMap::iterator f = m_keyBindTargetMap.find(key);
   if (f != m_keyBindTargetMap.end())
   {
      f->second->ExecuteBinding(key, down, time);
      return;
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cInput
//

///////////////////////////////////////

cInput::cInput()
 : m_modalListeners(this)
 , m_oldMouseState(0)
{
   memset(m_keyRepeats, 0, sizeof(m_keyRepeats));
}

///////////////////////////////////////

cInput::~cInput()
{
}

///////////////////////////////////////

tResult cInput::Init()
{
   SysSetCharEventCallback(OnSysCharEvent, reinterpret_cast<uint_ptr>(this));
   SysSetKeyEventCallback(OnSysKeyEvent, reinterpret_cast<uint_ptr>(this));
   SysSetMouseMoveCallback(OnSysMouseMove, reinterpret_cast<uint_ptr>(this));
   SysSetMouseEventCallback(OnSysMouseEvent, reinterpret_cast<uint_ptr>(this));

   AddInputListener(&m_modalListeners);

   return S_OK;
}

///////////////////////////////////////

tResult cInput::Term()
{
   RemoveInputListener(&m_modalListeners);

   SysSetCharEventCallback(NULL, 0);
   SysSetKeyEventCallback(NULL, 0);
   SysSetMouseMoveCallback(NULL, 0);
   SysSetMouseEventCallback(NULL, 0);

   UnbindAll();

   return S_OK;
}

///////////////////////////////////////

tResult cInput::AddInputListener(IInputListener * pListener)
{
   return Connect(pListener);
}

///////////////////////////////////////

tResult cInput::RemoveInputListener(IInputListener * pListener)
{
   return Disconnect(pListener);
}

///////////////////////////////////////

bool cInput::KeyIsDown(long key)
{
   if (key > -1 && key < kMaxKeys)
   {
      return m_keyRepeats[key] > 0;
   }
   else
   {
      return false;
   }
}

///////////////////////////////////////

void cInput::KeyBind(long key, const char * pszDownCmd, const char * pszUpCmd)
{
   cAutoIPtr<IInputKeyBindTarget> pKeyBindTarget(static_cast<IInputKeyBindTarget *>(new cScriptKeyBindTarget(pszDownCmd, pszUpCmd)));

   Bind(key, pKeyBindTarget);
}

///////////////////////////////////////

tResult cInput::KeyBind(long key, IInputKeyBindTarget * pKeyBindTarget)
{
   if (key < 0 || key >= kMaxKeys)
   {
      return E_INVALIDARG;
   }

   if (pKeyBindTarget == NULL)
   {
      return E_POINTER;
   }

   return Bind(key, pKeyBindTarget);
}

///////////////////////////////////////

void cInput::KeyUnbind(long key)
{
   Unbind(key);
}

///////////////////////////////////////

tResult cInput::PushModalListener(IInputModalListener * pModalListener)
{
   return m_modalListeners.PushModalListener(pModalListener);
}

///////////////////////////////////////

tResult cInput::PopModalListener()
{
   return m_modalListeners.PopModalListener();
}

///////////////////////////////////////

bool cInput::DispatchInputEvent(int x, int y, long key, bool down, double time)
{
   DebugMsgIfEx5(InputEvents, key != kMouseMove, "%3d %s (%3d, %3d) %f\n", key, down ? "down" : " up ", x, y, time);

   Assert((key >= 0) && (key < kMaxKeys));

   if (down)
   {
      m_keyRepeats[key]++;
   }
   else
   {
      m_keyRepeats[key] = 0;
   }

   int modifierKeys = kMK_None;
   if (m_keyRepeats[kAlt] > 0)
   {
      modifierKeys |= kMK_Alt;
   }
   if (m_keyRepeats[kCtrl] > 0)
   {
      modifierKeys |= kMK_Ctrl;
   }
   if (m_keyRepeats[kLShift] > 0)
   {
      modifierKeys |= kMK_LShift;
   }
   if (m_keyRepeats[kRShift] > 0)
   {
      modifierKeys |= kMK_RShift;
   }

   sInputEvent event;
   event.key = key;
   event.modifierKeys = modifierKeys;
   event.down = down;
   event.point = cPoint2<int>(x, y);
   event.time = time;

   bool bGotToModalListeners = false;

   tSinksIterator iter = BeginSinks();
   tSinksIterator end = EndSinks();
   for (; iter != end; ++iter)
   {
      if ((*iter)->OnInputEvent(&event))
      {
         if (!bGotToModalListeners)
         {
            m_modalListeners.CancelAll();
         }
         return true; // do no further processing for this key event
      }

      if (CTIsSameObject(*iter, &m_modalListeners))
      {
         bGotToModalListeners = true;
      }
   }

   return false;
}

///////////////////////////////////////

void cInput::OnSysCharEvent(tChar c, double time, uint_ptr userData)
{
   cInput * pThis = reinterpret_cast<cInput*>(userData);
   pThis->ReportKeyEvent(c, true, time);
   pThis->ReportKeyEvent(c, false, time);
}

///////////////////////////////////////

void cInput::OnSysKeyEvent(long key, bool down, double time, uint_ptr userData)
{
   cInput * pThis = reinterpret_cast<cInput*>(userData);
   pThis->ReportKeyEvent(key, down, time);
}

///////////////////////////////////////

void cInput::OnSysMouseMove(int x, int y, uint mouseState, double time, uint_ptr userData)
{
   cInput * pThis = reinterpret_cast<cInput*>(userData);
   // Up/down doesn't matter for mouse motion. Use false so that the
   // repeat count doesn't get incremented wildly.
   pThis->DispatchInputEvent(x, y, kMouseMove, false, time);
}

///////////////////////////////////////

void cInput::OnSysMouseEvent(int x, int y, uint mouseState, double time, uint_ptr userData)
{
   cInput * pThis = reinterpret_cast<cInput*>(userData);
   pThis->ReportMouseEvent(x, y, mouseState, time);
}

///////////////////////////////////////

void cInput::ReportKeyEvent(long key, bool down, double time)
{
   // TODO: Provide mouse position even for key events
   if (DispatchInputEvent(0, 0, key, down, time))
   {
      return;
   }

   ExecuteBinding(key, down, time);
}

///////////////////////////////////////

void cInput::ReportMouseEvent(int x, int y, uint mouseState, double time)
{
   if ((mouseState & kLMouseDown) && !(m_oldMouseState & kLMouseDown))
   {
      DispatchInputEvent(x, y, kMouseLeft, true, time);
   }
   else if (!(mouseState & kLMouseDown) && (m_oldMouseState & kLMouseDown))
   {
      DispatchInputEvent(x, y, kMouseLeft, false, time);
   }

   if ((mouseState & kRMouseDown) && !(m_oldMouseState & kRMouseDown))
   {
      DispatchInputEvent(x, y, kMouseRight, true, time);
   }
   else if (!(mouseState & kRMouseDown) && (m_oldMouseState & kRMouseDown))
   {
      DispatchInputEvent(x, y, kMouseRight, false, time);
   }

   if ((mouseState & kMMouseDown) && !(m_oldMouseState & kMMouseDown))
   {
      DispatchInputEvent(x, y, kMouseMiddle, true, time);
   }
   else if (!(mouseState & kMMouseDown) && (m_oldMouseState & kMMouseDown))
   {
      DispatchInputEvent(x, y, kMouseMiddle, false, time);
   }

   m_oldMouseState = mouseState;
}

///////////////////////////////////////

cInput::cInputModalListeners::cInputModalListeners(cInput * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

bool cInput::cInputModalListeners::OnInputEvent(const sInputEvent * pEvent)
{
   Assert(pEvent != NULL);
   Assert(m_pOuter != NULL);

   if (!m_modalListeners.empty())
   {
      cAutoIPtr<IInputModalListener> pModalListener(CTAddRef(m_modalListeners.top()));
      Assert(!!pModalListener);
      return pModalListener->OnInputEvent(pEvent);
   }

   return false;
}

///////////////////////////////////////

tResult cInput::cInputModalListeners::PushModalListener(IInputModalListener * pModalListener)
{
   if (pModalListener == NULL)
   {
      return E_POINTER;
   }
   m_modalListeners.push(CTAddRef(pModalListener));
   return S_OK;
}

///////////////////////////////////////

tResult cInput::cInputModalListeners::PopModalListener()
{
   if (m_modalListeners.empty())
   {
      return E_FAIL;
   }
   m_modalListeners.top()->Release();
   m_modalListeners.pop();
   return S_OK;
}

///////////////////////////////////////

void cInput::cInputModalListeners::CancelAll()
{
   while (!m_modalListeners.empty())
   {
      cAutoIPtr<IInputModalListener> pModalListener(m_modalListeners.top());
      m_modalListeners.pop();
      pModalListener->CancelMode();
   }
}

///////////////////////////////////////

tResult InputCreate()
{
   cAutoIPtr<IInput> pInput(static_cast<IInput*>(new cInput));
   if (!pInput)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IInput, pInput);
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

struct sTestKeyEvent
{
   long key;
   bool down;
   double time;
};

static bool operator ==(const sTestKeyEvent & a, const sTestKeyEvent & b)
{
   return (a.key == b.key)
      && (a.down == b.down)
      && (a.time == b.time);
}

class cTestKeyBindTarget : public cComObject<IMPLEMENTS(IInputKeyBindTarget)>
{
public:
   virtual void ExecuteBinding(long key, bool down, double time)
   {
      sTestKeyEvent keyEvent = { key, down, time };
      m_events.push_back(keyEvent);
   }

   size_t EventCount() const { return m_events.size(); }
   const sTestKeyEvent & Event(size_t index) { return m_events[index]; }
   std::vector<sTestKeyEvent>::const_iterator FirstEvent() const { return m_events.begin(); }
   std::vector<sTestKeyEvent>::const_iterator LastEvent() const { return m_events.end(); }

private:
   std::vector<sTestKeyEvent> m_events;
};

TEST_FIXTURE(cInput, KeyBinding)
{
   cAutoIPtr<cTestKeyBindTarget> pKeyBindTarget(new cTestKeyBindTarget);

   KeyBind('t', static_cast<IInputKeyBindTarget*>(pKeyBindTarget));

   static const sTestKeyEvent keyEvents[] =
   {
      { 't', true, .01 },
      { 't', false, .02 },
      { 'e', true, .03 },
      { 'e', false, .04 },
   };

   for (int i = 0; i < _countof(keyEvents); ++i)
   {
      const sTestKeyEvent & keyEvent = keyEvents[i];
      ReportKeyEvent(keyEvent.key, keyEvent.down, keyEvent.time);
   }

   CHECK_EQUAL(2, pKeyBindTarget->EventCount());
   CHECK(pKeyBindTarget->Event(0) == keyEvents[0]);
   CHECK(pKeyBindTarget->Event(1) == keyEvents[1]);
}

#endif

///////////////////////////////////////////////////////////////////////////////
