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
#include <vector>

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

   cScriptResults results;

   int nArgs = lua_gettop(L);

   if (nArgs > 0)
   {
   #ifdef DBGALLOC_MAPPED
   #undef new
   #endif
      void * pArgMem = alloca(nArgs * sizeof(cScriptVar));
      cScriptVar * pArgs = new(pArgMem) cScriptVar[nArgs];
   #ifdef DBGALLOC_MAPPED
   #define new DebugNew
   #endif

      for (int i = 0; i < nArgs; i++)
      {
         switch (lua_type(L, i + 1))
         {
            case LUA_TNUMBER:
            {
               pArgs[i].type = kNumber;
               pArgs[i].d = lua_tonumber(L, i + 1);
               break;
            }

            case LUA_TSTRING:
            {
               pArgs[i].type = kString;
               pArgs[i].psz = const_cast<char *>(lua_tostring(L, i + 1));
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
               pArgs[i].type = kEmpty;
               break;
            }
         }
      }

      (*pfn)(nArgs, pArgs, &results);

      lua_pop(L, nArgs);
   }
   else
   {
      (*pfn)(0, NULL, &results);
   }

   if (results.empty())
      return 0; // we are not returning any values on the stack

   for (cScriptResults::iterator iter = results.begin(); iter != results.end(); iter++)
   {
      switch ((*iter).type)
      {
         case kNumber:
         {
            lua_pushnumber(L, (*iter).d);
            break;
         }

         case kString:
         {
            lua_pushstring(L, (*iter).psz);
            break;
         }
      }
   }

   return results.size();
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

bool cScriptMachine::Init()
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

   return true;
}

///////////////////////////////////////

void cScriptMachine::Term()
{
   if (m_L != NULL)
      lua_close(m_L);
   m_L = NULL;
}

///////////////////////////////////////

bool cScriptMachine::ExecFile(const char * pszFile)
{
   if (m_L != NULL)
      return (lua_dofile(m_L, pszFile) == 0);
   else
      return false;
}

///////////////////////////////////////

bool cScriptMachine::ExecString(const char * pszCode)
{
   if (m_L != NULL)
      return (lua_dobuffer(m_L, pszCode, strlen(pszCode), pszCode) == 0);
   else
      return false;
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

bool cScriptMachine::AddFunction(const char * pszName, tScriptFn pfn)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushlightuserdata(m_L, (void *)pfn);
      lua_pushcclosure(m_L, LuaThunk, 1);
      lua_setglobal(m_L, pszName);
      return true;
   }
   return false;
}

///////////////////////////////////////

bool cScriptMachine::RemoveFunction(const char * pszName)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushnil(m_L);
      lua_setglobal(m_L, pszName);
      return true;
   }
   return false;
}

///////////////////////////////////////

bool cScriptMachine::GetVar(const char * pszName, cScriptVar * pValue)
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
   return bFound;
}

///////////////////////////////////////

bool cScriptMachine::GetVar(const char * pszName, double * pValue)
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
         return true;
      }
      lua_pop(m_L, 1);
   }
   return false;
}

///////////////////////////////////////

bool cScriptMachine::GetVar(const char * pszName, char * pValue, int cbMaxValue)
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
         return true;
      }
      lua_pop(m_L, 1);
   }
   return false;
}

///////////////////////////////////////

void cScriptMachine::SetVar(const char * pszName, double value)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushnumber(m_L, value);
      lua_setglobal(m_L, pszName);
   }
}

///////////////////////////////////////

void cScriptMachine::SetVar(const char * pszName, const char * pszValue)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushstring(m_L, pszValue);
      lua_setglobal(m_L, pszName);
   }
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

   static void CallThisFunction(int argc, const cScriptVar * argv, cScriptResults *)
   {
      gm_bCalled = true;
   }

   static void RemoveThisFunction(int, const cScriptVar *, cScriptResults *)
   {
      // does nothing
   }

   void TestCallFunction()
   {
      CPPUNIT_ASSERT(m_sm.AddFunction("CallThisFunction", CallThisFunction));
      gm_bCalled = false;
      CPPUNIT_ASSERT(m_sm.ExecString("CallThisFunction();"));
      CPPUNIT_ASSERT(gm_bCalled);
      m_sm.RemoveFunction("CallThisFunction");
   }

   void TestRemoveFunction()
   {
      CPPUNIT_ASSERT(m_sm.AddFunction("RemoveThisFunction", RemoveThisFunction));
      CPPUNIT_ASSERT(m_sm.ExecString("RemoveThisFunction();"));
      m_sm.RemoveFunction("RemoveThisFunction");
      CPPUNIT_ASSERT(!m_sm.ExecString("RemoveThisFunction();"));

      CPPUNIT_ASSERT(!m_sm.ExecString("ThisNameWillNotBeFoundSoThisCallShouldFail();"));
   }

   void TestGetNumber()
   {
      double value;

      m_sm.SetVar("foo", 123.456);
      CPPUNIT_ASSERT(m_sm.GetVar("foo", &value));
      CPPUNIT_ASSERT(value == 123.456);
   }

   void TestGetString()
   {
      char szValue[16];

      m_sm.SetVar("bar", "blah blah");
      CPPUNIT_ASSERT(m_sm.GetVar("bar", szValue, _countof(szValue)));
      CPPUNIT_ASSERT(strcmp(szValue, "blah blah") == 0);

      m_sm.SetVar("bar", "blah blah blah blah blah blah blah blah");
      CPPUNIT_ASSERT(m_sm.GetVar("bar", szValue, _countof(szValue)));
      CPPUNIT_ASSERT(strcmp(szValue, "blah blah blah ") == 0);
   }

   cScriptMachine m_sm;

public:
   virtual void setUp()
   {
      CPPUNIT_ASSERT(m_sm.Init());
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
