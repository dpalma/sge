///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPTVM_H
#define INCLUDED_SCRIPTVM_H

#include <cstdarg>
#include <vector>
#include "scriptvar.h"

#ifdef _MSC_VER
#pragma once
#endif

typedef struct lua_State lua_State;

// this is a class instead of a typedef for better forward-declaring elsewhere
class cScriptResults : public std::vector<cScriptVar>
{
};

typedef void (* tScriptFn)(int, const cScriptVar *, cScriptResults *);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScriptMachine
//

class cScriptMachine
{
public:
   cScriptMachine();
   ~cScriptMachine();

   bool Init();
   void Term();
   bool ExecFile(const char * pszFile);
   bool ExecString(const char * pszCode);
   void CallFunction(const char * pszName, const char * pszArgDesc = NULL, ...);
   void CallFunction(const char * pszName, const char * pszArgDesc, va_list args);
   bool AddFunction(const char * pszName, tScriptFn pfn);
   bool RemoveFunction(const char * pszName);

   bool GetVar(const char * pszName, cScriptVar * pValue);
   bool GetVar(const char * pszName, double * pValue);
   bool GetVar(const char * pszName, char * pValue, int cbMaxValue);
   void SetVar(const char * pszName, double value);
   void SetVar(const char * pszName, const char * pszValue);

private:
   lua_State * m_L;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPTVM_H
