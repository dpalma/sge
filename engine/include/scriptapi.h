///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCRIPTAPI_H
#define INCLUDED_SCRIPTAPI_H

/// @file scriptapi.h
/// Interface definitions for script language integration

#include "enginedll.h"
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
/// @typedef tScriptFn
/// @brief Function signature for functions to be exposed to script code

typedef int (* tScriptFn)(int, const cScriptVar *, int, cScriptVar *);

///////////////////////////////////////////////////////////////////////////////
/// @struct tScriptFn
/// @brief Pair that includes a script function pointer plus its name as exposed to script code

struct sScriptReg
{
   const char * pszName;
   tScriptFn pfn;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IScriptableFactory
//
/// @interface IScriptableFactory
/// @brief The factory interface for IScriptable objects
/// @remarks To expose a class for use in script code, register a factory
/// with the run-time interpreter using IScriptInterpreter#RegisterCustomClass. 
/// The factory manufactures instances of scriptable objects when an object is 
/// created in script code.

interface IScriptableFactory : IUnknown
{
   virtual tResult CreateInstance(void * * ppvInstance) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IScriptable
//
/// @interface IScriptable
/// @brief Implement this interface to expose a scriptable version of a
/// class's public interface to the run-time interpreter.

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
/// @interface IScriptInterpreter
/// @brief The script interpreter interface isolates game and engine
/// code from the details of the actual scripting language in use.

interface IScriptInterpreter : IUnknown
{
   /// @brief Execute a script code source file
   virtual tResult ExecFile(const char * pszFile) = 0;
   /// @brief Execute script code stored in a string variable
   virtual tResult ExecString(const char * pszCode) = 0;

   /// @brief Invoke a global script function
   /// @param pszName is the name of the function to invoke
   /// @param pszArgDesc is a string describing the arguments passed to the script 
   /// function: 'i' denotes an integer, 'f' denotes a floating point number, and 
   /// 's' denotes a string.
   /// @param ... is the variable list of arguments
   /// @code
   /// UseGlobal(pScriptInterpreter);
   /// pScriptInterpreter->CallFunction("Square", "ff", 100.0, 50.9);
   /// @endcode
   virtual void CallFunction(const char * pszName, const char * pszArgDesc = NULL, ...) = 0;
   /*!
    * @overload void CallFunction(const char * pszName, const char * pszArgDesc, va_list args);
    */
   virtual void CallFunction(const char * pszName, const char * pszArgDesc, va_list args) = 0;

   /// @brief Expose a global function to script code using the given name
   /// @param pszName is the name script code will use to invoke the function
   /// @param pfn is a pointer to the function's C++ implementation
   virtual tResult AddFunction(const char * pszName, tScriptFn pfn) = 0;
   /// @brief Revoke a previously registered function
   /// @param pszName is the name of the function to be removed
   virtual tResult RemoveFunction(const char * pszName) = 0;

   virtual tResult GetGlobal(const char * pszName, cScriptVar * pValue) = 0;
   virtual tResult GetGlobal(const char * pszName, double * pValue) = 0;
   virtual tResult GetGlobal(const char * pszName, char * pValue, int cbMaxValue) = 0;
   virtual void SetGlobal(const char * pszName, double value) = 0;
   virtual void SetGlobal(const char * pszName, const char * pszValue) = 0;

   virtual tResult RegisterCustomClass(const tChar * pszClassName, IScriptableFactory * pFactory) = 0;
   virtual tResult RevokeCustomClass(const tChar * pszClassName) = 0;
};

///////////////////////////////////////
/// Create the singleton script interpreter object

ENGINE_API void ScriptInterpreterCreate();

///////////////////////////////////////
/// Register a global function with the script interpreter
/// @see @ref tScriptFn

ENGINE_API tResult ScriptAddFunction(const char * pszName, tScriptFn pfn);

///////////////////////////////////////
/// Register many global functions with the script interpreter at once
/// @see @ref tScriptFn

ENGINE_API tResult ScriptAddFunctions(const sScriptReg * pFns, uint nFns);

///////////////////////////////////////

class ENGINE_API cScriptAutoAddFunction
{
public:
   cScriptAutoAddFunction(const char * pszName, tScriptFn pfn);
};

#define PROTOTYPE_SCRIPTFUNCTION(fnName) \
   extern int (fnName)(int, const cScriptVar *, int, cScriptVar *);

#define AUTOADD_SCRIPTFUNCTION(name, pfn) \
   PROTOTYPE_SCRIPTFUNCTION(pfn) \
   cScriptAutoAddFunction MAKE_UNIQUE(g_auto##name##ScriptFn)(#name, pfn)

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCRIPTAPI_H
