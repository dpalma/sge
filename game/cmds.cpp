///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "sys.h"
#include "script.h"
#include "scriptvar.h"
#include "inputapi.h"
#include "keys.h"
#include "globalobj.h"

#include "dbgalloc.h" // must be last header

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
      UseGlobal(Input);
      pInput->KeyBind(Name2Key(pszKeyName), pszDownCmd, pszUpCmd);
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(unbind)
{
   if (ScriptArgc() == 1 && ScriptArgv(0).type == kString)
   {
      UseGlobal(Input);
      pInput->KeyUnbind(Name2Key(ScriptArgv(0).psz));
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(LogEnableChannel)
{
   if (ScriptArgc() == 1 && ScriptArgIsString(0))
   {
      LogEnableChannel(ScriptArgAsString(0), true);
   }
   else if (ScriptArgc() == 2 && ScriptArgIsString(0) && ScriptArgIsNumber(1))
   {
      LogEnableChannel(ScriptArgAsString(0), ScriptArgAsNumber(1) ? true : false);
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(quit)
{
   SysQuit();
   return 0;
}

///////////////////////////////////////////////////////////////////////////////
