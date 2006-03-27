///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_LUAUTIL_H
#define INCLUDED_LUAUTIL_H

#include "comtools.h"

#include <cstdarg>

#ifdef _MSC_VER
#pragma once
#endif

typedef struct lua_State lua_State;

class cMultiVar;
typedef cMultiVar tScriptVar;

F_DECLARE_INTERFACE(IScriptable);

///////////////////////////////////////////////////////////////////////////////

const int kLuaMaxArgs = 16;
const int kLuaMaxResults = 8;

///////////////////////////////////////////////////////////////////////////////

void LuaGetArg(lua_State * L, int index, tScriptVar * pArg);

int LuaPushResults(lua_State * L, int nResults, tScriptVar * results);

int LuaPublishObject(lua_State * L, IScriptable * pInstance);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_LUAUTIL_H
