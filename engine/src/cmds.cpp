///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guiapi.h"
#include "inputapi.h"
#include "scriptapi.h"
#include "scriptvar.h"

#include "globalobj.h"
#include "keys.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#define ScriptArgIsString(iArg) (argv[iArg].type == kString)
#define ScriptArgIsNumber(iArg) (argv[iArg].type == kNumber)

#define ScriptArgAsString(iArg) (argv[iArg].psz)
#define ScriptArgAsNumber(iArg) (argv[iArg].d)

///////////////////////////////////////////////////////////////////////////////

static const struct
{
   const char * pszKeyName;
   uint keyCode;
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

static const size_t g_nNamedKeys = _countof(g_keyNames);

///////////////////////////////////////////////////////////////////////////////

const char * Key2Name(long key)
{
   for (int i = 0; i < g_nNamedKeys; i++)
   {
      if (g_keyNames[i].keyCode == key)
      {
         return g_keyNames[i].pszKeyName;
      }
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////

long Name2Key(const char * pszKeyName)
{
   for (int i = 0; i < g_nNamedKeys; i++)
   {
      if (stricmp(g_keyNames[i].pszKeyName, pszKeyName) == 0)
      {
         return g_keyNames[i].keyCode;
      }
   }
   if (*pszKeyName > 0)
   {
      return *pszKeyName;
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

int BindKey(int argc, const cScriptVar * argv, 
            int nMaxResults, cScriptVar * pResults)
{
   const char * pszKeyName = NULL;
   const char * pszDownCmd = NULL;
   const char * pszUpCmd = NULL;

   if (argc == 2)
   {
      pszKeyName = ScriptArgIsString(0) ? ScriptArgAsString(0) : NULL;
      pszDownCmd = ScriptArgIsString(1) ? ScriptArgAsString(1) : NULL;
   }
   else if (argc == 3)
   {
      pszKeyName = ScriptArgIsString(0) ? ScriptArgAsString(0) : NULL;
      pszDownCmd = ScriptArgIsString(1) ? ScriptArgAsString(1) : NULL;
      pszUpCmd = ScriptArgIsString(2) ? ScriptArgAsString(2) : NULL;
   }

   if (pszKeyName != NULL && (pszDownCmd != NULL || pszUpCmd != NULL))
   {
      UseGlobal(Input);
      if (!!pInput)
      {
         pInput->KeyBind(Name2Key(pszKeyName), pszDownCmd, pszUpCmd);
      }
   }

   return 0;
}

AUTOADD_SCRIPTFUNCTION(bind, BindKey);

///////////////////////////////////////////////////////////////////////////////

int UnbindKey(int argc, const cScriptVar * argv, 
              int nMaxResults, cScriptVar * pResults)
{
   if (argc == 1 && ScriptArgIsString(0))
   {
      UseGlobal(Input);
      if (!!pInput)
      {
         pInput->KeyUnbind(Name2Key(ScriptArgAsString(0)));
      }
   }

   return 0;
}

AUTOADD_SCRIPTFUNCTION(unbind, UnbindKey);

///////////////////////////////////////////////////////////////////////////////

int LogEnableChannel(int argc, const cScriptVar * argv, 
                     int nMaxResults, cScriptVar * pResults)
{
   if (argc == 1 && ScriptArgIsString(0))
   {
      techlog.EnableChannel(ScriptArgAsString(0), true);
   }
   else if (argc == 2 && ScriptArgIsString(0) && ScriptArgIsNumber(1))
   {
      techlog.EnableChannel(ScriptArgAsString(0), ScriptArgAsNumber(1) ? true : false);
   }
   return 0;
}

AUTOADD_SCRIPTFUNCTION(LogChannel, LogEnableChannel);

///////////////////////////////////////////////////////////////////////////////

int LoadGUI(int argc, const cScriptVar * argv, 
            int nMaxResults, cScriptVar * pResults)
{
   if (argc == 1 && ScriptArgIsString(0))
   {
      UseGlobal(GUIContext);
      if (!!pGUIContext)
      {
         if (pGUIContext->LoadFromString(ScriptArgAsString(0)) == 0)
         {
            if (pGUIContext->LoadFromResource(ScriptArgAsString(0)) == 0)
            {
               DebugMsg1("Loading GUI definitions from %s\n", ScriptArgAsString(0));
            }
         }
      }
   }

   return 0;
}

AUTOADD_SCRIPTFUNCTION(LoadGUI, LoadGUI);

///////////////////////////////////////////////////////////////////////////////

int ToggleGUIDebugInfo(int argc, const cScriptVar * argv, 
                       int nMaxResults, cScriptVar * pResults)
{
   tGUIPoint placement(0,0);
   tGUIColor color(tGUIColor::White);

   if (argc == 2 
      && ScriptArgIsNumber(0) 
      && ScriptArgIsNumber(1))
   {
      placement = tGUIPoint(ScriptArgAsNumber(0), ScriptArgAsNumber(1));
   }
   else if (argc == 3 
      && ScriptArgIsNumber(0) 
      && ScriptArgIsNumber(1) 
      && ScriptArgIsString(2))
   {
      placement = tGUIPoint(ScriptArgAsNumber(0), ScriptArgAsNumber(1));
      GUIStyleParseColor(ScriptArgAsString(2), &color);
   }
   else if (argc == 5 
      && ScriptArgIsNumber(0) 
      && ScriptArgIsNumber(1) 
      && ScriptArgIsNumber(2) 
      && ScriptArgIsNumber(3) 
      && ScriptArgIsNumber(4))
   {
      placement = tGUIPoint(ScriptArgAsNumber(0), ScriptArgAsNumber(1));
      color = tGUIColor(ScriptArgAsNumber(2), ScriptArgAsNumber(3), ScriptArgAsNumber(4));
   }

   UseGlobal(GUIContext);
   if (!!pGUIContext)
   {
      if (pGUIContext->ShowDebugInfo(placement, color) == S_FALSE)
      {
         pGUIContext->HideDebugInfo();
      }
   }

   return 0;
}

AUTOADD_SCRIPTFUNCTION(ToggleGUIDebugInfo, ToggleGUIDebugInfo);

///////////////////////////////////////////////////////////////////////////////

sScriptReg cmds[] =
{
   { "bind", BindKey },
   { "unbind", UnbindKey },
   { "LogChannel", LogEnableChannel },
   { "LoadGUI", LoadGUI },
   { "ToggleGUIDebugInfo", ToggleGUIDebugInfo },
};

///////////////////////////////////////////////////////////////////////////////
