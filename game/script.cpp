///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "script.h"
#include "scriptvm.h"

#include <cstdarg>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

cScriptMachine g_scriptMachine;
bool g_bScriptMachineInitialized = false;

///////////////////////////////////////////////////////////////////////////////
// queue mechanism avoids use of a global std::vector or something so that
// the queue can be used at static initialization time

struct sScriptAddFunctionDefer
{
   const char * pszName;
   tScriptFn pfn;
   sScriptAddFunctionDefer * pNext;
};

sScriptAddFunctionDefer * g_pDeferredAddFunctions = NULL;

// just in case the queue never gets consumed
struct sDeferredAddFunctionsAutoCleanup
{
   ~sDeferredAddFunctionsAutoCleanup()
   {
      sScriptAddFunctionDefer * p = g_pDeferredAddFunctions;
      while (p != NULL)
      {
         g_pDeferredAddFunctions = g_pDeferredAddFunctions->pNext;
         delete p;
         p = g_pDeferredAddFunctions;
      }
   }
} g_deferredAddFunctionsAutoCleanup;

///////////////////////////////////////////////////////////////////////////////

void ScriptInit(int) // @TODO: remove the 'int' parameter
{
   if (!g_scriptMachine.Init())
   {
      DebugMsg("WARNING: Error initializing script machine\n");
      return;
   }

   g_bScriptMachineInitialized = true;

   sScriptAddFunctionDefer * p = g_pDeferredAddFunctions;
   while (p != NULL)
   {
      // it's a good thing g_L != NULL at this point otherwise this
      // function would add onto g_pDeferredAddFunctions again!
      ScriptAddFunction(p->pszName, p->pfn);
      p = p->pNext;
   }
}

///////////////////////////////////////////////////////////////////////////////

void ScriptTerm()
{
   g_bScriptMachineInitialized = false;
   g_scriptMachine.Term();
}

///////////////////////////////////////////////////////////////////////////////

bool ScriptExecFile(const char * pszFile)
{
   return g_scriptMachine.ExecFile(pszFile);
}

///////////////////////////////////////////////////////////////////////////////

bool ScriptExecString(const char * pszCode)
{
   return g_scriptMachine.ExecString(pszCode);
}

///////////////////////////////////////////////////////////////////////////////

void ScriptCallFunction(const char * pszName, const char * pszArgDesc, ...)
{
   va_list args;
   va_start(args, pszArgDesc);
   g_scriptMachine.CallFunction(pszName, pszArgDesc, args);
   va_end(args);
}

///////////////////////////////////////////////////////////////////////////////

void ScriptAddFunction(const char * pszName, tScriptFn pfn)
{
   if (g_bScriptMachineInitialized)
   {
      g_scriptMachine.AddFunction(pszName, pfn);
   }
   else
   {
      // simple queue to support adding at static init time
      sScriptAddFunctionDefer * p = new sScriptAddFunctionDefer;
      p->pszName = pszName;
      p->pfn = pfn;
      p->pNext = g_pDeferredAddFunctions;
      g_pDeferredAddFunctions = p;
   }
}

///////////////////////////////////////////////////////////////////////////////

void ScriptRemoveFunction(const char * pszName)
{
   g_scriptMachine.RemoveFunction(pszName);
}

///////////////////////////////////////////////////////////////////////////////

bool ScriptGetVar(const char * pszName, cScriptVar * pValue)
{
   return g_scriptMachine.GetVar(pszName, pValue);
}

///////////////////////////////////////////////////////////////////////////////

bool ScriptGetVar(const char * pszName, double * pValue)
{
   return g_scriptMachine.GetVar(pszName, pValue);
}

///////////////////////////////////////////////////////////////////////////////

bool ScriptGetVar(const char * pszName, char * pValue, int cbMaxValue)
{
   return g_scriptMachine.GetVar(pszName, pValue, cbMaxValue);
}

///////////////////////////////////////////////////////////////////////////////

void ScriptSetVar(const char * pszName, double value)
{
   g_scriptMachine.SetVar(pszName, value);
}

///////////////////////////////////////////////////////////////////////////////

void ScriptSetVar(const char * pszName, const char * pszValue)
{
   g_scriptMachine.SetVar(pszName, pszValue);
}

///////////////////////////////////////////////////////////////////////////////
