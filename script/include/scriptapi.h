///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPTAPI_H
#define INCLUDED_SCRIPTAPI_H

#include "comtools.h"
#include <cstdarg>

#ifdef _MSC_VER
#pragma once
#endif

class cScriptVar;

F_DECLARE_INTERFACE(IScriptable);
F_DECLARE_INTERFACE(IScriptableFactory);
F_DECLARE_INTERFACE(IScriptInterpreter);

///////////////////////////////////////////////////////////////////////////////

typedef int (* tScriptFn)(int, const cScriptVar *, int, cScriptVar *);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IScriptableFactory
//

interface IScriptableFactory : IUnknown
{
   virtual tResult CreateInstance(void * * ppvInstance) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IScriptable
//

interface IScriptable : IUnknown
{
   virtual tResult Invoke(const char * pszMethodName,
                          int nArgs, const cScriptVar * pArgs,
                          int nMaxResults, cScriptVar * pResults) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IScriptInterpreter
//

interface IScriptInterpreter : IUnknown
{
   virtual tResult Init() = 0;
   virtual void Term() = 0;

   virtual tResult ExecFile(const char * pszFile) = 0;
   virtual tResult ExecString(const char * pszCode) = 0;

   virtual void CallFunction(const char * pszName, const char * pszArgDesc = NULL, ...) = 0;
   virtual void CallFunction(const char * pszName, const char * pszArgDesc, va_list args) = 0;
   virtual tResult AddFunction(const char * pszName, tScriptFn pfn) = 0;
   virtual tResult RemoveFunction(const char * pszName) = 0;

   virtual tResult GetGlobal(const char * pszName, cScriptVar * pValue) = 0;
   virtual tResult GetGlobal(const char * pszName, double * pValue) = 0;
   virtual tResult GetGlobal(const char * pszName, char * pValue, int cbMaxValue) = 0;
   virtual void SetGlobal(const char * pszName, double value) = 0;
   virtual void SetGlobal(const char * pszName, const char * pszValue) = 0;

   virtual tResult RegisterCustomClass(const tChar * pszClassName, IScriptableFactory * pFactory) = 0;
   virtual tResult RevokeCustomClass(const tChar * pszClassName) = 0;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPTAPI_H
