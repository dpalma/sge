///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "scriptvm.h"

extern "C"
{
#include <lualib.h>
#include <lauxlib.h>
}

#include <cstdio>
#include <cstring>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
// modified version of Lua print function

int LuaPrintEx(lua_State * L)
{
   int n = lua_gettop(L);  /* number of arguments */
   int i;
   lua_getglobal(L, "tostring");
   for (i=1; i<=n; i++)
   {
      const char *s;
      lua_pushvalue(L, -1);  /* function to be called */
      lua_pushvalue(L, i);   /* value to print */
      lua_call(L, 1, 1);
      s = lua_tostring(L, -1);  /* get result */
      if (s == NULL)
         luaL_error(L, "`tostring' must return a string to `print'");

      if (i>1)
         fputs("\t", stdout);
      fputs(s, stdout);

#ifndef NDEBUG
      if (i>1)
         DebugPrintf(0, 0, "\t");
      DebugPrintf(0, 0, s);
#endif

      lua_pop(L, 1);  /* pop result */
   }
   fputs("\n", stdout);
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

int LuaThunk(lua_State * L)
{
   tScriptFn pfn = (tScriptFn)lua_touserdata(L, lua_upvalueindex(1));
   if (pfn == NULL)
      return 0; // no C function to call

   const int kMaxResults = 8;
   cScriptVar results[kMaxResults];
   int nResults = 0;

   int nArgs = lua_gettop(L);

   if (nArgs > 0)
   {
      const int kMaxArgs = 16;
      cScriptVar args[kMaxArgs];

      if (nArgs > kMaxArgs)
         nArgs = kMaxArgs;

      for (int i = 0; i < nArgs; i++)
      {
         switch (lua_type(L, i + 1))
         {
            case LUA_TNUMBER:
            {
               args[i].type = kNumber;
               args[i].d = lua_tonumber(L, i + 1);
               break;
            }

            case LUA_TSTRING:
            {
               args[i].type = kString;
               args[i].psz = const_cast<char *>(lua_tostring(L, i + 1));
               break;
            }

            /*
            case LUA_TTABLE:
            {
               lua_pushnil(L);
               while (lua_next(L, i + 1))
               {
                  const char * key = lua_tostring(L, -2);
                  // value at -1
                  // ...
                  lua_pop(L, 1);
               }
               break;
            }
            */

            default:
            {
               args[i].type = kEmpty;
               break;
            }
         }
      }

      nResults = (*pfn)(nArgs, args, _countof(results), results);

      lua_pop(L, nArgs);
   }
   else
   {
      nResults = (*pfn)(0, NULL, _countof(results), results);
   }

   if (nResults == 0)
      return 0; // we are not returning any values on the stack

   for (int i = 0; i < nResults; i++)
   {
      switch (results[i].type)
      {
         case kNumber:
         {
            lua_pushnumber(L, results[i].d);
            break;
         }

         case kString:
         {
            lua_pushstring(L, results[i].psz);
            break;
         }
      }
   }

   return nResults;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cScriptMachine
//

///////////////////////////////////////

cScriptMachine::cScriptMachine()
 : m_L(NULL)
{
}

///////////////////////////////////////

cScriptMachine::~cScriptMachine()
{
   DebugMsgIf(m_L != NULL, "WARNING: script machine destructor called before Term!\n");
}

///////////////////////////////////////

tResult cScriptMachine::Init()
{
   Assert(m_L == NULL);
   if (m_L != NULL)
   {
      return E_FAIL;
   }

   m_L = lua_open();
   if (m_L == NULL)
   {
      return E_FAIL;
   }

   luaopen_base(m_L);
   luaopen_table(m_L);
   luaopen_io(m_L);
   luaopen_string(m_L);
   luaopen_math(m_L);
#ifndef NDEBUG
   luaopen_debug(m_L);
#endif
   luaopen_loadlib(m_L);

   lua_register(m_L, "print", LuaPrintEx);

   return S_OK;
}

///////////////////////////////////////

void cScriptMachine::Term()
{
   if (m_L != NULL)
      lua_close(m_L);
   m_L = NULL;
}

///////////////////////////////////////

tResult cScriptMachine::ExecFile(const char * pszFile)
{
   if ((m_L != NULL) && (lua_dofile(m_L, pszFile) == 0))
      return S_OK;
   else
      return E_FAIL;
}

///////////////////////////////////////

tResult cScriptMachine::ExecString(const char * pszCode)
{
   if ((m_L != NULL) && (lua_dobuffer(m_L, pszCode, strlen(pszCode), pszCode) == 0))
      return S_OK;
   else
      return E_FAIL;
}

///////////////////////////////////////

void cScriptMachine::CallFunction(const char * pszName, const char * pszArgDesc, ...)
{
   Assert(pszName != NULL);
   va_list args;
   va_start(args, pszArgDesc);
   CallFunction(pszName, pszArgDesc, args);
   va_end(args);
}

///////////////////////////////////////

void cScriptMachine::CallFunction(const char * pszName, const char * pszArgDesc, va_list args)
{
   if (m_L != NULL)
   {
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
                  DebugMsg1("Unknown arg type spec %c\n", *p);
                  break;
               }
            }
         }
         lua_getglobal(m_L, pszName);
         lua_call(m_L, strlen(pszArgDesc), 0);
      }
      else
      {
         lua_getglobal(m_L, pszName);
         lua_call(m_L, 0, 0);
      }
   }
}

///////////////////////////////////////

tResult cScriptMachine::AddFunction(const char * pszName, tScriptFn pfn)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushlightuserdata(m_L, (void *)pfn);
      lua_pushcclosure(m_L, LuaThunk, 1);
      lua_setglobal(m_L, pszName);
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cScriptMachine::RemoveFunction(const char * pszName)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushnil(m_L);
      lua_setglobal(m_L, pszName);
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cScriptMachine::GetGlobal(const char * pszName, cScriptVar * pValue)
{
   Assert(pszName != NULL);
   Assert(pValue != NULL);
   bool bFound = false;
   if (m_L != NULL)
   {
      lua_getglobal(m_L, pszName);
      switch (lua_type(m_L, -1))
      {
         case LUA_TNUMBER:
         {
            pValue->type = kNumber;
            pValue->d = lua_tonumber(m_L, -1);
            bFound = true;
            break;
         }

         case LUA_TSTRING:
         {
            pValue->type = kString;
            pValue->psz = const_cast<char *>(lua_tostring(m_L, -1));
            bFound = true;
            break;
         }

         default:
         {
            pValue->type = kEmpty;
            break;
         }
      }
      lua_pop(m_L, 1);
   }
   return bFound ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cScriptMachine::GetGlobal(const char * pszName, double * pValue)
{
   Assert(pszName != NULL);
   Assert(pValue != NULL);
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
   return E_FAIL;
}

///////////////////////////////////////

tResult cScriptMachine::GetGlobal(const char * pszName, char * pValue, int cbMaxValue)
{
   Assert(pszName != NULL);
   Assert(pValue != NULL);
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
   return E_FAIL;
}

///////////////////////////////////////

void cScriptMachine::SetGlobal(const char * pszName, double value)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushnumber(m_L, value);
      lua_setglobal(m_L, pszName);
   }
}

///////////////////////////////////////

void cScriptMachine::SetGlobal(const char * pszName, const char * pszValue)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushstring(m_L, pszValue);
      lua_setglobal(m_L, pszName);
   }
}

///////////////////////////////////////

tResult cScriptMachine::RegisterCustomClass(const tChar * pszClassName, IScriptableFactory * pFactory)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cScriptMachine::RevokeCustomClass(const tChar * pszClassName)
{
   return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cScriptMachineTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cScriptMachineTests);
      CPPUNIT_TEST(TestCallFunction);
      CPPUNIT_TEST(TestRemoveFunction);
      CPPUNIT_TEST(TestGetNumber);
      CPPUNIT_TEST(TestGetString);
   CPPUNIT_TEST_SUITE_END();

private:
   static bool gm_bCalled;

   static int CallThisFunction(int argc, const cScriptVar * argv, int, cScriptVar *)
   {
      gm_bCalled = true;
      return 0;
   }

   static int RemoveThisFunction(int, const cScriptVar *, int, cScriptVar *)
   {
      // does nothing
      return 0;
   }

   void TestCallFunction()
   {
      CPPUNIT_ASSERT(m_sm.AddFunction("CallThisFunction", CallThisFunction) == S_OK);
      gm_bCalled = false;
      CPPUNIT_ASSERT(m_sm.ExecString("CallThisFunction();") == S_OK);
      CPPUNIT_ASSERT(gm_bCalled);
      m_sm.RemoveFunction("CallThisFunction");
   }

   void TestRemoveFunction()
   {
      CPPUNIT_ASSERT(m_sm.AddFunction("RemoveThisFunction", RemoveThisFunction) == S_OK);
      CPPUNIT_ASSERT(m_sm.ExecString("RemoveThisFunction();") == S_OK);
      m_sm.RemoveFunction("RemoveThisFunction");
      CPPUNIT_ASSERT(m_sm.ExecString("RemoveThisFunction();") != S_OK);

      CPPUNIT_ASSERT(m_sm.ExecString("ThisNameWillNotBeFoundSoThisCallShouldFail();") != S_OK);
   }

   void TestGetNumber()
   {
      double value;

      m_sm.SetGlobal("foo", 123.456);
      CPPUNIT_ASSERT(m_sm.GetGlobal("foo", &value) == S_OK);
      CPPUNIT_ASSERT(value == 123.456);
   }

   void TestGetString()
   {
      char szValue[16];

      m_sm.SetGlobal("bar", "blah blah");
      CPPUNIT_ASSERT(m_sm.GetGlobal("bar", szValue, _countof(szValue)) == S_OK);
      CPPUNIT_ASSERT(strcmp(szValue, "blah blah") == 0);

      m_sm.SetGlobal("bar", "blah blah blah blah blah blah blah blah");
      CPPUNIT_ASSERT(m_sm.GetGlobal("bar", szValue, _countof(szValue)) == S_OK);
      CPPUNIT_ASSERT(strcmp(szValue, "blah blah blah ") == 0);
   }

   cScriptMachine m_sm;

public:
   virtual void setUp()
   {
      CPPUNIT_ASSERT(m_sm.Init() == S_OK);
   }

   virtual void tearDown()
   {
      m_sm.Term();
   }
};

bool cScriptMachineTests::gm_bCalled = false;

CPPUNIT_TEST_SUITE_REGISTRATION(cScriptMachineTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
