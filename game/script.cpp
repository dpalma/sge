///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "script.h"

#include "globalobj.h"

#include <cstdarg>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

bool ScriptExecFile(const char * pszFile)
{
   UseGlobal(ScriptInterpreter);
   return pScriptInterpreter->ExecFile(pszFile) == S_OK;
}

///////////////////////////////////////////////////////////////////////////////

bool ScriptExecString(const char * pszCode)
{
   UseGlobal(ScriptInterpreter);
   return pScriptInterpreter->ExecString(pszCode) == S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void ScriptCallFunction(const char * pszName, const char * pszArgDesc, ...)
{
   UseGlobal(ScriptInterpreter);
   va_list args;
   va_start(args, pszArgDesc);
   pScriptInterpreter->CallFunction(pszName, pszArgDesc, args);
   va_end(args);
}

///////////////////////////////////////////////////////////////////////////////
