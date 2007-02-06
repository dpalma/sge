///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "input.h"

#include "platform/keys.h"
#include "platform/sys.h"
#include "script/scriptapi.h"

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
// CLASS: cKeyBindings
//

///////////////////////////////////////

cKeyBindings::cKeyBindings()
{
   Assert(_countof(m_keyDownBindings) == kMaxKeys);
   Assert(_countof(m_keyDownBindings) == _countof(m_keyUpBindings));
   memset(m_keyDownBindings, 0, sizeof(m_keyDownBindings));
   memset(m_keyUpBindings, 0, sizeof(m_keyUpBindings));
}

///////////////////////////////////////

cKeyBindings::~cKeyBindings()
{
   UnbindAll();
}

///////////////////////////////////////

void cKeyBindings::Bind(long key, const char * pszDownCmd, const char * pszUpCmd)
{
   Assert(key > -1 && key < kMaxKeys);

   Assert(m_keyDownBindings[toupper(key)] == NULL);
   key = KeyGetBindable(key);

   Unbind(key);

   if (pszDownCmd != NULL && *pszDownCmd != 0)
   {
      m_keyDownBindings[key] = new char[strlen(pszDownCmd) + 1];
      strcpy(m_keyDownBindings[key], pszDownCmd);
   }

   if (pszUpCmd != NULL && *pszUpCmd != 0)
   {
      m_keyUpBindings[key] = new char[strlen(pszUpCmd) + 1];
      strcpy(m_keyUpBindings[key], pszUpCmd);
   }
}

///////////////////////////////////////

void cKeyBindings::Unbind(long key)
{
   Assert(key > -1 && key < kMaxKeys);

   key = KeyGetBindable(key);

   delete [] m_keyDownBindings[key];
   m_keyDownBindings[key] = NULL;

   delete [] m_keyUpBindings[key];
   m_keyUpBindings[key] = NULL;
}

///////////////////////////////////////

void cKeyBindings::UnbindAll()
{
   Assert(_countof(m_keyDownBindings) == _countof(m_keyUpBindings));
   for (int i = 0; i < _countof(m_keyDownBindings); i++)
   {
      delete [] m_keyDownBindings[i];
      m_keyDownBindings[i] = NULL;
      delete [] m_keyUpBindings[i];
      m_keyUpBindings[i] = NULL;
   }
}

///////////////////////////////////////

const char * cKeyBindings::GetDownBinding(long key) const
{
   Assert(key > -1 && key < kMaxKeys);
   if (key > -1 && key < kMaxKeys)
      return m_keyDownBindings[KeyGetBindable(key)];
   else
      return NULL;
}

///////////////////////////////////////

const char * cKeyBindings::GetUpBinding(long key) const
{
   Assert(key > -1 && key < kMaxKeys);
   if (key > -1 && key < kMaxKeys)
      return m_keyUpBindings[KeyGetBindable(key)];
   else
      return NULL;
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

   AddInputListener(&m_modalListeners, kILP_ModalListeners);

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

tResult cInput::AddInputListener(IInputListener * pListener, int priority)
{
   return cConnectionPointEx<cInput, IInputListener>::AddSink(pListener, priority);
}

///////////////////////////////////////

tResult cInput::RemoveInputListener(IInputListener * pListener)
{
   return cConnectionPointEx<cInput, IInputListener>::RemoveSink(pListener);
}

///////////////////////////////////////

void cInput::SortSinks(tSinksIterator first, tSinksIterator last)
{
   std::stable_sort(first, last, std::greater< std::pair<int, IInputListener*> >());
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
   Bind(key, pszDownCmd, pszUpCmd);
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

tResult cInput::SetModalListenerPriority(int priority)
{
   RemoveInputListener(&m_modalListeners);
   AddInputListener(&m_modalListeners, priority);
   return S_OK;
}

///////////////////////////////////////

bool cInput::DispatchInputEvent(int x, int y, long key, bool down, double time)
{
   DebugMsgEx5(InputEvents, "%3d %s (%3d, %3d) %f\n", key, down ? "down" : " up ", x, y, time);

   Assert((key >= 0) && (key < kMaxKeys));

   if (down)
   {
      m_keyRepeats[key]++;
   }
   else
   {
      m_keyRepeats[key] = 0;
   }

   sInputEvent event;
   event.key = key;
   event.down = down;
   event.point = cVec2<int>(x, y);
   event.time = time;

   bool bGotToModalListeners = false;

   tSinksIterator iter = BeginSinks();
   tSinksIterator end = EndSinks();
   for (; iter != end; ++iter)
   {
      if (iter->second->OnInputEvent(&event))
      {
         if (!bGotToModalListeners)
         {
            m_modalListeners.CancelAll();
         }
         return true; // do no further processing for this key event
      }

      if (CTIsSameObject(iter->second, &m_modalListeners))
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

   const char * pBinding = down ? GetDownBinding(key) : GetUpBinding(key);
   if (pBinding != NULL)
   {
      UseGlobal(ScriptInterpreter);
      pScriptInterpreter->ExecString(pBinding);
   }
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
