///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPT_H
#define INCLUDED_SCRIPT_H

#include "scriptapi.h"

#ifdef _MSC_VER
#pragma once
#endif

class cScriptVar;

///////////////////////////////////////////////////////////////////////////////

void ScriptInit();
void ScriptTerm();
bool ScriptExecFile(const char * pszFile);
bool ScriptExecString(const char * pszCode);
void ScriptCallFunction(const char * pszName,
                        const char * pszArgDesc = NULL, ...);
void ScriptAddFunction(const char * pszName, tScriptFn pfn);

struct sScriptAutoAddFunction
{
   sScriptAutoAddFunction(const char * pszName, tScriptFn pfn)
   {
      ScriptAddFunction(pszName, pfn);
   }
};

#define SCRIPT_FUNCTION_NAME(name) name

#define SCRIPT_DEFINE_FUNCTION_NO_AUTOADD(name) \
   int SCRIPT_FUNCTION_NAME(name)(int argc, const cScriptVar * argv, int nMaxResults, cScriptVar * pResults)

#define SCRIPT_DEFINE_FUNCTION(name) \
   SCRIPT_DEFINE_FUNCTION_NO_AUTOADD(name); \
   static sScriptAutoAddFunction g_AutoAdd##name(#name, SCRIPT_FUNCTION_NAME(name)); \
   SCRIPT_DEFINE_FUNCTION_NO_AUTOADD(name)

#define ScriptArgc() argc
#define ScriptArgv(arg) argv[arg]

#define ScriptArgIsString(arg) (ScriptArgv(arg).type == kString)
#define ScriptArgIsNumber(arg) (ScriptArgv(arg).type == kNumber)

#define ScriptArgAsString(arg) (ScriptArgv(arg).psz)
#define ScriptArgAsNumber(arg) (ScriptArgv(arg).d)

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPT_H
