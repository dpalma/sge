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
// queueing mechanism cannot use a global std::vector or anything like that
// so that the queue can be used at static initialization time

struct sFunctionQueueEntry
{
   const char * pszName;
   tScriptFn pfn;
   sFunctionQueueEntry * pNext;
};

sFunctionQueueEntry * g_pQueuedFunctions = NULL;

// just in case the queue never gets consumed
struct sQueuedFunctionsAutoCleanup
{
   ~sQueuedFunctionsAutoCleanup()
   {
      sFunctionQueueEntry * p = g_pQueuedFunctions;
      while (p != NULL)
      {
         g_pQueuedFunctions = g_pQueuedFunctions->pNext;
         delete p;
         p = g_pQueuedFunctions;
      }
   }
} g_queuedFunctionsAutoCleanup;

///////////////////////////////////////////////////////////////////////////////

void ScriptInit()
{
   if (FAILED(g_scriptMachine.Init()))
   {
      DebugMsg("WARNING: Error initializing script machine\n");
      return;
   }

   g_bScriptMachineInitialized = true;

   while (g_pQueuedFunctions != NULL)
   {
      ScriptAddFunction(g_pQueuedFunctions->pszName, g_pQueuedFunctions->pfn);
      sFunctionQueueEntry * p = g_pQueuedFunctions;
      g_pQueuedFunctions = g_pQueuedFunctions->pNext;
      delete p;
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
   return g_scriptMachine.ExecFile(pszFile) == S_OK;
}

///////////////////////////////////////////////////////////////////////////////

bool ScriptExecString(const char * pszCode)
{
   return g_scriptMachine.ExecString(pszCode) == S_OK;
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
      sFunctionQueueEntry * p = new sFunctionQueueEntry;
      p->pszName = pszName;
      p->pfn = pfn;
      p->pNext = g_pQueuedFunctions;
      g_pQueuedFunctions = p;
   }
}

///////////////////////////////////////////////////////////////////////////////
