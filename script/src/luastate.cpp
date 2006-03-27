///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "luastate.h"
#include "luautil.h"

#include "multivar.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

extern "C"
{
#include <lualib.h>
#include <lauxlib.h>
}

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(LuaState);

#define LocalMsg(msg)                  DebugMsgEx(LuaState,msg)
#define LocalMsg1(msg,a1)              DebugMsgEx1(LuaState,msg,(a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(LuaState,msg,(a1),(a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(LuaState,(cond),msg)
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(LuaState,(cond),msg,(a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(LuaState,(cond),msg,(a1),(a2))

///////////////////////////////////////////////////////////////////////////////

static const int kLuaNoError = 0;


///////////////////////////////////////////////////////////////////////////////

static int LuaThunkFunction(lua_State * L)
{
   tScriptFn pfn = (tScriptFn)lua_touserdata(L, lua_upvalueindex(1));
   if (pfn == NULL)
   {
      return 0; // no C function to call
   }

   int nArgsOnStack = lua_gettop(L);
   int nArgs = Min(kLuaMaxArgs, nArgsOnStack);

   tScriptVar results[kLuaMaxResults];
   int result = -1;

   if (nArgs > 0)
   {
      tScriptVar args[kLuaMaxArgs];

      for (int i = 0; i < nArgs; i++)
      {
         LuaGetArg(L, i + 1, &args[i]);
      }

      lua_pop(L, nArgsOnStack);

      result = (*pfn)(nArgs, args, kLuaMaxResults, results);
   }
   else
   {
      result = (*pfn)(0, NULL, kLuaMaxResults, results);
   }

   if (result <= 0)
   {
      return 0; // we are not returning any values on the stack
   }

   int nResultsPushed = LuaPushResults(L, result, results);
   lua_checkstack(L, nResultsPushed);
   return nResultsPushed;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLuaState
//

///////////////////////////////////////

cLuaState::cLuaState()
 : m_L(NULL)
{
}

///////////////////////////////////////

cLuaState::~cLuaState()
{
   Close();
}

///////////////////////////////////////

bool cLuaState::Open(uint libs)
{
   Assert(m_L == NULL);
   if (m_L != NULL)
   {
      return false;
   }

   m_L = lua_open();
   if (m_L == NULL)
   {
      return false;
   }

   static const struct
   {
      uint luaLibFlag;
      int (*pfnLuaLibOpen)(lua_State *);
   }
   luaLibOpenFns[] =
   {
      { kLSL_Base, luaopen_base },
      { kLSL_Table, luaopen_table },
      { kLSL_IO, luaopen_io },
      { kLSL_String, luaopen_string },
      { kLSL_Math, luaopen_math },
#ifndef NDEBUG
      { kLSL_Debug, luaopen_debug },
#endif
      { kLSL_Loadlib, luaopen_loadlib },
   };

   for (int i = 0; i < _countof(luaLibOpenFns); i++)
   {
      uint libFlag = luaLibOpenFns[i].luaLibFlag;
      if ((libs & libFlag) == libFlag)
      {
         int result = (*luaLibOpenFns[i].pfnLuaLibOpen)(m_L);
      }
   }

   return true;
}

///////////////////////////////////////

void cLuaState::Close()
{
   if (m_L != NULL)
   {
      lua_close(m_L);
      m_L = NULL;
   }
}

///////////////////////////////////////

tResult cLuaState::DoFile(const tChar * pszFile)
{
   if (m_L == NULL)
   {
      return E_FAIL;
   }
#ifdef _UNICODE
   uint tempSize = (wcslen(pszFile) + 1) * sizeof(char);
   char * pszTemp = reinterpret_cast<char*>(alloca(tempSize));
   wcstombs(pszTemp, pszFile, tempSize);
   if (lua_dofile(m_L, pszTemp) == 0)
#else
   if (lua_dofile(m_L, pszFile) == 0)
#endif
   {
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cLuaState::DoString(const char * pszCode)
{
   if (m_L != NULL
      && lua_dobuffer(m_L, pszCode, strlen(pszCode), pszCode) == 0)
   {
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cLuaState::CallFunction(const char * pszName, const char * pszArgDesc, va_list args)
{
   if (pszName == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   if (m_L != NULL)
   {
      int luaResult = kLuaNoError;

      if (pszArgDesc && *pszArgDesc)
      {
         for (const char * p = pszArgDesc; *p != 0; p++)
         {
            switch (*p)
            {
               case 'i':
               {
                  int i = va_arg(args, int);
                  lua_pushnumber(m_L, i);
                  break;
               }

               case 'f':
               {
                  double f = va_arg(args, double);
                  lua_pushnumber(m_L, f);
                  break;
               }

               case 's':
               {
                  const char * psz = va_arg(args, const char *);
                  lua_pushstring(m_L, psz);
                  break;
               }

               default:
               {
                  WarnMsg1("Unknown arg type spec %c\n", *p);
                  break;
               }
            }
           lua_checkstack(m_L, 1);
         }
         lua_getglobal(m_L, pszName);
         luaResult = lua_pcall(m_L, strlen(pszArgDesc), 0, 0);
      }
      else
      {
         lua_getglobal(m_L, pszName);
         luaResult = lua_pcall(m_L, 0, 0, 0);
      }

      static const tResult resultMap[] =
      {
         S_OK, // kLuaNoError (zero)
         E_FAIL, // LUA_ERRRUN
         E_FAIL, // LUA_ERRFILE
         E_FAIL, // LUA_ERRSYNTAX
         E_OUTOFMEMORY, // LUA_ERRMEM
         E_UNEXPECTED, // LUA_ERRERR
      };

      Assert(luaResult >= 0 && luaResult < _countof(resultMap));

      result = resultMap[luaResult];
   }

   return result;
}

///////////////////////////////////////

tResult cLuaState::SetGlobal(const char * pszName, const tScriptVar & value)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cLuaState::SetGlobal(const char * pszName, double value)
{
   if (pszName == NULL)
   {
      return E_POINTER;
   }
   if (m_L == NULL)
   {
      return E_FAIL;
   }
   lua_pushnumber(m_L, value);
   lua_setglobal(m_L, pszName);
   return S_OK;
}

///////////////////////////////////////

tResult cLuaState::SetGlobal(const char * pszName, const char * pszValue)
{
   if (pszName == NULL || pszValue == NULL)
   {
      return E_POINTER;
   }
   if (m_L == NULL)
   {
      return E_FAIL;
   }
   lua_pushstring(m_L, pszValue);
   lua_setglobal(m_L, pszName);
   return S_OK;
}

///////////////////////////////////////

tResult cLuaState::SetGlobal(const char * pszName, tScriptFn pfn)
{
   if (pszName == NULL || pfn == NULL)
   {
      return E_POINTER;
   }
   if (m_L == NULL)
   {
      return E_FAIL;
   }
   // The old-fashioned C-style cast is required for gcc, which distinguishes
   // between pointers-to-functions and pointers-to-objects. The real solution
   // would be to have a struct that contains the function pointer as a field.
   // This would require a Lua "__gc" handler to free the struct.
   lua_pushlightuserdata(m_L, (void*)pfn);
   lua_pushcclosure(m_L, LuaThunkFunction, 1);
   lua_setglobal(m_L, pszName);
   return S_OK;
}

///////////////////////////////////////

tResult cLuaState::GetGlobal(const char * pszName, tScriptVar * pValue) const
{
   if (pszName == NULL || pValue == NULL)
   {
      return E_POINTER;
   }
   bool bFound = false;
   if (m_L != NULL)
   {
      lua_getglobal(m_L, pszName);
      switch (lua_type(m_L, -1))
      {
         case LUA_TNUMBER:
         {
            pValue->Assign(lua_tonumber(m_L, -1));
            bFound = true;
            break;
         }

         case LUA_TSTRING:
         {
            pValue->Assign(lua_tostring(m_L, -1));
            bFound = true;
            break;
         }

         default:
         {
            pValue->Clear();
            break;
         }
      }
      lua_pop(m_L, 1);
   }
   return bFound ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cLuaState::GetGlobal(const char * pszName, double * pValue) const
{
   if (pszName == NULL || pValue == NULL)
   {
      return E_POINTER;
   }
   if (m_L != NULL)
   {
      lua_getglobal(m_L, pszName);
      if (lua_type(m_L, -1) == LUA_TNUMBER)
      {
         *pValue = lua_tonumber(m_L, -1);
         lua_pop(m_L, 1);
         return S_OK;
      }
      lua_pop(m_L, 1);
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cLuaState::GetGlobal(const char * pszName, char * pValue, int cbMaxValue) const
{
   if (pszName == NULL || pValue == NULL)
   {
      return E_POINTER;
   }
   if (m_L != NULL)
   {
      lua_getglobal(m_L, pszName);
      int type = lua_type(m_L, -1);
      if (type == LUA_TNUMBER || type == LUA_TSTRING)
      {
         strncpy(pValue, lua_tostring(m_L, -1), cbMaxValue);
         pValue[cbMaxValue - 1] = 0;
         lua_pop(m_L, 1);
         return S_OK;
      }
      lua_pop(m_L, 1);
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cLuaState::RemoveGlobal(const char * pszName)
{
   if (pszName == NULL)
   {
      return E_POINTER;
   }
   if (m_L == NULL)
   {
      return E_FAIL;
   }
   lua_pushnil(m_L);
   lua_setglobal(m_L, pszName);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLuaStateTests
//

class cLuaStateTests
{
protected:
   cLuaStateTests();
   ~cLuaStateTests();

   static bool gm_bCalled;

   static int CallThisFunction(int argc, const tScriptVar * argv, int, tScriptVar *);
   static int RemoveThisFunction(int, const tScriptVar *, int, tScriptVar *);

   cLuaState m_luaState;
};

////////////////////////////////////////

cLuaStateTests::cLuaStateTests()
{
   m_luaState.Open();
}

////////////////////////////////////////

cLuaStateTests::~cLuaStateTests()
{
   m_luaState.Close();
}

////////////////////////////////////////

bool cLuaStateTests::gm_bCalled = false;

////////////////////////////////////////

int cLuaStateTests::CallThisFunction(int argc, const tScriptVar * argv, int, tScriptVar *)
{
   gm_bCalled = true;
   return 0;
}

////////////////////////////////////////

int cLuaStateTests::RemoveThisFunction(int, const tScriptVar *, int, tScriptVar *)
{
   // does nothing
   return 0;
}

////////////////////////////////////////

TEST_F(cLuaStateTests, CallFunction)
{
   CHECK(m_luaState.SetGlobal("CallThisFunction", CallThisFunction) == S_OK);
   gm_bCalled = false;
   CHECK(m_luaState.DoString("CallThisFunction();") == S_OK);
   CHECK(gm_bCalled);
   CHECK(m_luaState.RemoveGlobal("CallThisFunction") == S_OK);
}

////////////////////////////////////////

TEST_F(cLuaStateTests, RemoveFunction)
{
   CHECK(m_luaState.SetGlobal("RemoveThisFunction", RemoveThisFunction) == S_OK);
   CHECK(m_luaState.DoString("RemoveThisFunction();") == S_OK);
   CHECK(m_luaState.RemoveGlobal("RemoveThisFunction") == S_OK);
   CHECK(m_luaState.DoString("RemoveThisFunction();") != S_OK);

   CHECK(m_luaState.DoString("ThisNameWillNotBeFoundSoThisCallShouldFail();") != S_OK);
}

////////////////////////////////////////

TEST_F(cLuaStateTests, GetNumber)
{
   double value;
   m_luaState.SetGlobal("foo", 123.456);
   CHECK(m_luaState.GetGlobal("foo", &value) == S_OK);
   CHECK_EQUAL(value, 123.456);
}

////////////////////////////////////////

TEST_F(cLuaStateTests, GetString)
{
   char szValue[16];

   m_luaState.SetGlobal("bar", "blah blah");
   CHECK(m_luaState.GetGlobal("bar", szValue, _countof(szValue)) == S_OK);
   CHECK(strcmp(szValue, "blah blah") == 0);

   m_luaState.SetGlobal("bar", "blah blah blah blah blah blah blah blah");
   CHECK(m_luaState.GetGlobal("bar", szValue, _countof(szValue)) == S_OK);
   CHECK(strcmp(szValue, "blah blah blah ") == 0);
}

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
