///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "script.h"
#include "scriptvar.h"
#include "input.h"
#include "keys.h"
#include "connptimpl.h"
#include <string.h>
#include <stdlib.h>
#include <list>
#include <locale>
#include <algorithm>

#include "dbgalloc.h" // must be last header

#if !defined(NDEBUG) && defined(_MSC_VER)
EXTERN_C DECLSPEC_DLLIMPORT int CDECL _CrtIsValidHeapPointer(const void * userData);
#else
#define _CrtIsValidHeapPointer(userData) (true)
#endif

LOG_DEFINE_CHANNEL(KeyEvent);

///////////////////////////////////////////////////////////////////////////////

static void KeyBind(long key, const char * pszKeyDownCmd, const char * pszKeyUpCmd);
static void KeyUnbind(long key);

const int kMaxKeys = 256;
ulong g_keyRepeats[kMaxKeys] = { 0 };
char * g_keyDownBindings[kMaxKeys] = { 0 };
char * g_keyUpBindings[kMaxKeys] = { 0 };
typedef std::list<IInputListener *> tInputListeners;
tInputListeners g_inputListeners;

///////////////////////////////////////////////////////////////////////////////

inline const char * KeyGetDownBinding(long key)
{
   Assert(key > -1 && key < kMaxKeys);
   if (key > -1 && key < kMaxKeys)
      return g_keyDownBindings[key];
   else
      return NULL;
}

inline const char * KeyGetUpBinding(long key)
{
   Assert(key > -1 && key < kMaxKeys);
   if (key > -1 && key < kMaxKeys)
      return g_keyUpBindings[key];
   else
      return NULL;
}

inline long KeyGetBindable(long key)
{
   // not allowed to bind uppercase keys
   if (isalpha(key))
   {
      Assert(KeyGetDownBinding(toupper(key)) == NULL);
      return tolower(key);
   }
   else
   {
      return key;
   }
}

///////////////////////////////////////////////////////////////////////////////

static const struct
{
   const char * pszKeyName;
   eKeyCode keyCode;
}
g_keyNames[] =
{
   { "F1", kF1 },
   { "F2", kF2 },
   { "F3", kF3 },
   { "F4", kF4 },
   { "F5", kF5 },
   { "F6", kF6 },
   { "F7", kF7 },
   { "F8", kF8 },
   { "F9", kF9 },
   { "F10", kF10 },
   { "F11", kF11 },
   { "F12", kF12 },
   { "Insert", kInsert },
   { "Delete", kDelete },
   { "Home", kHome },
   { "End", kEnd },
   { "PageUp", kPageUp },
   { "PageDown", kPageDown },
   { "Tab", kTab },
   { "Backspace", kBackspace },
   { "Enter", kEnter },
   { "Ctrl", kCtrl },
   { "LeftShift", kLShift },
   { "RightShift", kRShift },
   { "Alt", kAlt },
   { "Up", kUp },
   { "Down", kDown },
   { "Left", kLeft },
   { "Right", kRight },
   { "Escape", kEscape },
   { "Esc", kEscape },
   { "Space", kSpace },
   { "Pause", kPause },
   { "MouseLeft", kMouseLeft },
   { "MouseMiddle", kMouseMiddle },
   { "MouseRight", kMouseRight },
   { "MouseWheelUp", kMouseWheelUp },
   { "MouseWheelDown", kMouseWheelDown },
   { "MouseMove", kMouseMove },
};

///////////////////////////////////////////////////////////////////////////////

static const char * Key2Name(long key)
{
   static const int nNamedKeys = sizeof(g_keyNames) / sizeof(*g_keyNames);
   for (int i = 0; i < nNamedKeys; i++)
   {
      if (g_keyNames[i].keyCode == key)
         return g_keyNames[i].pszKeyName;
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////

static long Name2Key(const char * pszKeyName)
{
   static const int nNamedKeys = sizeof(g_keyNames) / sizeof(*g_keyNames);
   for (int i = 0; i < nNamedKeys; i++)
   {
      if (stricmp(g_keyNames[i].pszKeyName, pszKeyName) == 0)
         return g_keyNames[i].keyCode;
   }
   if (*pszKeyName > 0)
      return *pszKeyName;
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool KeyIsDown(long key)
{
   if (key > -1 && key < kMaxKeys)
      return g_keyRepeats[key] > 0;
   else
      return false;
}

///////////////////////////////////////////////////////////////////////////////

void KeyEvent(long key, bool down, double time)
{
#ifndef NDEBUG
   const char * pszKey = Key2Name(key);
   if (pszKey)
      DebugMsgEx2(KeyEvent, "Key%s: %s\n", down ? "Down" : "Up", pszKey);
   else
      DebugMsgEx2(KeyEvent, "Key%s: %d\n", down ? "Down" : "Up", key);
#endif

   if (key < 0 || key >= kMaxKeys)
   {
      DebugMsg2("WARNING: out of range key %d %s\n",
         key, down ? "pressed" : "released");
      return;
   }

   if (down)
   {
      g_keyRepeats[key]++;
   }
   else
   {
      g_keyRepeats[key] = 0;
   }

   // iterate in reverse order so the most recently added listener gets first crack
   tInputListeners::reverse_iterator iter;
   for (iter = g_inputListeners.rbegin(); iter != g_inputListeners.rend(); iter++)
   {
      if ((*iter)->OnKeyEvent(key, down, time))
      {
         return; // do no further processing for this key event
      }
   }

   key = KeyGetBindable(key);

   if (down)
   {
      const char * p = KeyGetDownBinding(key);
      if (p != NULL)
      {
         ScriptExecString(p);
      }
   }
   else
   {
      const char * p = KeyGetUpBinding(key);
      if (p != NULL)
      {
         ScriptExecString(p);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

void MouseEvent(int x, int y, uint mouseState, double time)
{
   static uint oldMouseState = 0;

   bool bListenerShortCircuit = false;

   tInputListeners::iterator iter;
   for (iter = g_inputListeners.begin(); iter != g_inputListeners.end(); iter++)
   {
      if ((*iter)->OnMouseEvent(x, y, mouseState, time))
      {
         bListenerShortCircuit = true;
         break;
      }
   }

   if (!bListenerShortCircuit)
   {
      // Up/down doesn't matter for mouse motion. Use false so that the
      // repeat count doesn't get incremented wildly.
      KeyEvent(kMouseMove, false, time);

      if ((mouseState & kLMouseDown) && !(oldMouseState & kLMouseDown))
         KeyEvent(kMouseLeft, true, time);
      else if (!(mouseState & kLMouseDown) && (oldMouseState & kLMouseDown))
         KeyEvent(kMouseLeft, false, time);

      if ((mouseState & kRMouseDown) && !(oldMouseState & kRMouseDown))
         KeyEvent(kMouseRight, true, time);
      else if (!(mouseState & kRMouseDown) && (oldMouseState & kRMouseDown))
         KeyEvent(kMouseRight, false, time);

      if ((mouseState & kMMouseDown) && !(oldMouseState & kMMouseDown))
         KeyEvent(kMouseMiddle, true, time);
      else if (!(mouseState & kMMouseDown) && (oldMouseState & kMMouseDown))
         KeyEvent(kMouseMiddle, false, time);
   }

   oldMouseState = mouseState;
}

///////////////////////////////////////////////////////////////////////////////

static void KeyBind(long key, const char * pszKeyDownCmd, const char * pszKeyUpCmd)
{
   Assert(key > -1 && key < kMaxKeys);

   key = KeyGetBindable(key);

   KeyUnbind(key);

   if (pszKeyDownCmd != NULL && *pszKeyDownCmd != 0)
   {
      g_keyDownBindings[key] = new char[strlen(pszKeyDownCmd) + 1];
      strcpy(g_keyDownBindings[key], pszKeyDownCmd);
   }

   if (pszKeyUpCmd != NULL && *pszKeyUpCmd != 0)
   {
      g_keyUpBindings[key] = new char[strlen(pszKeyUpCmd) + 1];
      strcpy(g_keyUpBindings[key], pszKeyUpCmd);
   }
}

///////////////////////////////////////////////////////////////////////////////

static void KeyUnbind(long key)
{
   Assert(key > -1 && key < kMaxKeys);

   key = KeyGetBindable(key);

   delete [] g_keyDownBindings[key];
   g_keyDownBindings[key] = NULL;

   delete [] g_keyUpBindings[key];
   g_keyUpBindings[key] = NULL;
}

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(bind)
{
   const char * pszKeyName = NULL;
   const char * pszDownCmd = NULL;
   const char * pszUpCmd = NULL;

   if (ScriptArgc() == 2)
   {
      pszKeyName = ScriptArgIsString(0) ? ScriptArgAsString(0) : NULL;
      pszDownCmd = ScriptArgIsString(1) ? ScriptArgAsString(1) : NULL;
   }
   else if (ScriptArgc() == 3)
   {
      pszKeyName = ScriptArgIsString(0) ? ScriptArgAsString(0) : NULL;
      pszDownCmd = ScriptArgIsString(1) ? ScriptArgAsString(1) : NULL;
      pszUpCmd = ScriptArgIsString(2) ? ScriptArgAsString(2) : NULL;
   }

   if (pszKeyName != NULL && (pszDownCmd != NULL || pszUpCmd != NULL))
   {
      KeyBind(Name2Key(pszKeyName), pszDownCmd, pszUpCmd);
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(unbind)
{
   if (ScriptArgc() == 1 && ScriptArgv(0).type == kString)
   {
      KeyUnbind(Name2Key(ScriptArgv(0).psz));
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

void InputAddListener(IInputListener * pListener)
{
   Assert(pListener != NULL);
   add_interface(g_inputListeners, pListener);
}

///////////////////////////////////////////////////////////////////////////////

bool InputRemoveListener(IInputListener * pListener)
{
   Assert(pListener != NULL);
   return remove_interface(g_inputListeners, pListener);
}

///////////////////////////////////////////////////////////////////////////////

void InputRemoveAllListeners()
{
   std::for_each(g_inputListeners.begin(), g_inputListeners.end(), CTInterfaceMethodRef(&IUnknown::Release));
   g_inputListeners.clear();
}

///////////////////////////////////////////////////////////////////////////////

void InputInit()
{
#if !defined(NDEBUG) && defined(_WIN32)
   Assert(_countof(g_keyDownBindings) == kMaxKeys);
   Assert(_countof(g_keyDownBindings) == _countof(g_keyUpBindings));
   for (int i = 0; i < _countof(g_keyDownBindings); i++)
   {
      Assert(_CrtIsValidHeapPointer(g_keyDownBindings[i]) || g_keyDownBindings[i] == NULL);
      Assert(_CrtIsValidHeapPointer(g_keyUpBindings[i]) || g_keyUpBindings[i] == NULL);
   }
#endif
}

///////////////////////////////////////////////////////////////////////////////

void InputTerm()
{
   Assert(_countof(g_keyDownBindings) == kMaxKeys);
   Assert(_countof(g_keyDownBindings) == _countof(g_keyUpBindings));
   for (int i = 0; i < _countof(g_keyDownBindings); i++)
   {
      delete [] g_keyDownBindings[i];
      g_keyDownBindings[i] = NULL;
      delete [] g_keyUpBindings[i];
      g_keyUpBindings[i] = NULL;
   }

   InputRemoveAllListeners();
}

///////////////////////////////////////////////////////////////////////////////
