///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_LUASTATE_H
#define INCLUDED_LUASTATE_H

#include <cstdarg>

#ifdef _MSC_VER
#pragma once
#endif

typedef long tResult;

typedef struct lua_State lua_State;

class cMultiVar;
typedef cMultiVar tScriptVar;

typedef int (* tScriptFn)(int, const tScriptVar *, int, tScriptVar *);

///////////////////////////////////////////////////////////////////////////////

enum eLuaStandardLibs
{
   kLSL_NONE      = 0,
   kLSL_Base      = (1 << 0),
   kLSL_Table     = (1 << 1),
   kLSL_IO        = (1 << 2),
   kLSL_String    = (1 << 3),
   kLSL_Math      = (1 << 4),
   kLSL_Debug     = (1 << 5),
   kLSL_Loadlib   = (1 << 6),
   kLSL_OS        = (1 << 7),
   kLSL_ALL       = ~0u
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLuaState
//

class cLuaState
{
public:
   cLuaState();
   ~cLuaState();

   bool Open(uint libs = kLSL_ALL);
   void Close();

   tResult DoFile(const char * pszFile);
   tResult DoString(const char * pszCode);

   tResult CallFunction(const char * pszName, const char * pszArgDesc, va_list args);

   tResult SetGlobal(const char * pszName, const tScriptVar & value);
   tResult SetGlobal(const char * pszName, double value);
   tResult SetGlobal(const char * pszName, const char * pszValue);

   tResult SetGlobal(const char * pszName, tScriptFn pfn);

   tResult GetGlobal(const char * pszName, tScriptVar * pValue) const;
   tResult GetGlobal(const char * pszName, double * pValue) const;
   tResult GetGlobal(const char * pszName, char * pValue, int cbMaxValue) const;

   tResult RemoveGlobal(const char * pszName);

   lua_State * AccessLuaState() { return m_L; }
   const lua_State * AccessLuaState() const { return m_L; }

private:
   lua_State * m_L;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_LUASTATE_H
