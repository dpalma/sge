///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPT_H
#define INCLUDED_SCRIPT_H

#ifdef _MSC_VER
#pragma once
#endif

class cScriptVar;
class cScriptResults;
typedef void (* tScriptFn)(int, const cScriptVar *, cScriptResults *);

///////////////////////////////////////////////////////////////////////////////

void ScriptInit(int stackSize = 0);
void ScriptTerm();
bool ScriptExecFile(const char * pszFile);
bool ScriptExecString(const char * pszCode);
void ScriptCallFunction(const char * pszName,
                        const char * pszArgDesc = NULL, ...);
void ScriptAddFunction(const char * pszName, tScriptFn pfn);
void ScriptRemoveFunction(const char * pszName);

struct sScriptAutoAddFunction
{
   sScriptAutoAddFunction(const char * pszName, tScriptFn pfn)
   {
      ScriptAddFunction(pszName, pfn);
   }
};

#define SCRIPT_FUNCTION_NAME(name) name

#define SCRIPT_DEFINE_FUNCTION_NO_AUTOADD(name) \
   void SCRIPT_FUNCTION_NAME(name)(int argc, const cScriptVar * argv, cScriptResults * pResults)

#define SCRIPT_DEFINE_FUNCTION(name) \
   void SCRIPT_FUNCTION_NAME(name)(int, const cScriptVar *, cScriptResults *); \
   static sScriptAutoAddFunction g_AutoAdd##name(#name, SCRIPT_FUNCTION_NAME(name)); \
   void SCRIPT_FUNCTION_NAME(name)(int argc, const cScriptVar * argv, cScriptResults * pResults)

#define ScriptArgc() argc
#define ScriptArgv(arg) argv[arg]

#define ScriptArgIsString(arg) (ScriptArgv(arg).type == kString)
#define ScriptArgIsNumber(arg) (ScriptArgv(arg).type == kNumber)

#define ScriptArgAsString(arg) (ScriptArgv(arg).psz)
#define ScriptArgAsNumber(arg) (ScriptArgv(arg).d)

#define ScriptAddResult(result) (pResults->push_back(cScriptVar((result))))

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPT_H
