///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPTVM_H
#define INCLUDED_SCRIPTVM_H

#include <cstdarg>
#include "scriptapi.h"
#include "scriptvar.h"

#ifdef _MSC_VER
#pragma once
#endif

typedef struct lua_State lua_State;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScriptMachine
//

class cScriptMachine : public cComObject<IMPLEMENTS(IScriptInterpreter)>
{
public:
   cScriptMachine();
   ~cScriptMachine();

   virtual void DeleteThis() {}

   virtual tResult Init();
   virtual void Term();

   virtual tResult ExecFile(const char * pszFile);
   virtual tResult ExecString(const char * pszCode);

   virtual void CallFunction(const char * pszName, const char * pszArgDesc = NULL, ...);
   virtual void CallFunction(const char * pszName, const char * pszArgDesc, va_list args);
   virtual tResult AddFunction(const char * pszName, tScriptFn pfn);
   virtual tResult RemoveFunction(const char * pszName);

   virtual tResult GetGlobal(const char * pszName, cScriptVar * pValue);
   virtual tResult GetGlobal(const char * pszName, double * pValue);
   virtual tResult GetGlobal(const char * pszName, char * pValue, int cbMaxValue);
   virtual void SetGlobal(const char * pszName, double value);
   virtual void SetGlobal(const char * pszName, const char * pszValue);

   virtual tResult RegisterCustomClass(const tChar * pszClassName, IScriptableFactory * pFactory);
   virtual tResult RevokeCustomClass(const tChar * pszClassName);

private:
   lua_State * m_L;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPTVM_H
