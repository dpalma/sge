///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "luainterp.h"
#include "scriptvar.h"

extern "C"
{
#include <lualib.h>
#include <lauxlib.h>
}

#include <cstdio>
#include <cstring>

#ifdef HAVE_CPPUNIT
#include <ctime>
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

const int kMaxArgs = 16;
const int kMaxResults = 8;

///////////////////////////////////////////////////////////////////////////////

#if 0
static DWORD CallThiscall(const void * args, size_t sz, void * object, DWORD func)
{
   DWORD retval;
   __asm
   {
      mov   ecx, sz       // get size of buffer
      mov   esi, args     // get buffer
      sub   esp, ecx      // allocate stack space
      mov   edi, esp      // start of destination stack frame
      shr   ecx, 2        // convert size of buffer from bytes to dwords
      rep   movsd         // copy it
      mov   ecx, object   // set "this"
      call  [func]        // call the function
      mov   retval, eax   // save the return value
   }
   return retval;
}
#endif

///////////////////////////////////////////////////////////////////////////////

static int LuaGarbageCollectInterface(lua_State * L)
{
   IUnknown * pUnk = static_cast<IUnknown *>(lua_unboxpointer(L, 1));
   Assert(pUnk != NULL);
   SafeRelease(pUnk);
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int LuaThunkInvoke(lua_State * L)
{
   // the name of the method to call is at the top of the stack
   const char * pszMethodName = lua_tostring(L, lua_upvalueindex(1));

   // the "this" pointer is at the bottom of the stack
   IScriptable * pInstance = static_cast<IScriptable *>(lua_unboxpointer(L, 1));

   // Subtract one to exclude the "this" pointer
   int nArgs = lua_gettop(L) - 1;

   if (nArgs > kMaxArgs)
      nArgs = kMaxArgs;

   cScriptVar results[kMaxResults];
   int result = -1;

   if (nArgs > 0)
   {
      cScriptVar args[kMaxArgs];

      for (int i = 0, iArg = -1; i < nArgs; i++, iArg--)
      {
         switch (lua_type(L, iArg))
         {
            case LUA_TNUMBER:
            {
               args[i] = lua_tonumber(L, iArg);
               break;
            }

            case LUA_TSTRING:
            {
               args[i] = const_cast<char *>(lua_tostring(L, iArg));
               break;
            }

            default:
            {
               args[i].type = kEmpty;
               DebugMsg2("Arg %d of unsupported type %s\n", iArg, lua_typename(L, lua_type(L, iArg)));
               break;
            }
         }
      }

      lua_pop(L, nArgs);

      result = pInstance->Invoke(pszMethodName, nArgs, args, kMaxResults, results);
   }
   else
   {
      result = pInstance->Invoke(pszMethodName, 0, NULL, kMaxResults, results);
   }

   if (FAILED(result))
   {
      DebugMsg2("IScriptable[0x%08X]->Invoke(%s, ...) failed\n", pInstance, pszMethodName);
      return 0;
   }

   Assert(result <= kMaxResults);

   for (int i = 0; i < result; i++)
   {
      switch (results[i].type)
      {
         case kNumber:
         {
            lua_pushnumber(L, results[i]);
            break;
         }

         case kString:
         {
            lua_pushstring(L, results[i]);
            break;
         }
      }
   }

   // on success, result is the number of return values
   return result;
}

///////////////////////////////////////////////////////////////////////////////

static int LuaIndex(lua_State * L)
{
   // pop the user data associated with the tag method (IScriptable* pointer)
   // leave the method name on the stack to be an upvalue for the thunk function
   lua_pushcclosure(L, LuaThunkInvoke, 1);
   return 1;
}

///////////////////////////////////////////////////////////////////////////////
// returns the number of items on the stack when the function exits

static int LuaPublishObject(lua_State * L, IScriptable * pInstance)
{
   lua_boxpointer(L, pInstance);
   pInstance->AddRef();

   lua_newtable(L);

   lua_pushliteral(L, "__gc");
   lua_pushcclosure(L, LuaGarbageCollectInterface, 0);
   lua_rawset(L, -3);

   lua_pushliteral(L, "__index");
   lua_pushcclosure(L, LuaIndex, 0);
   lua_rawset(L, -3);

   lua_setmetatable(L, -2);

   return 1;
}

///////////////////////////////////////////////////////////////////////////////

static int LuaConstructObject(lua_State * L)
{
   IScriptableFactory * pFactory =
      static_cast<IScriptableFactory *>(lua_touserdata(L, lua_upvalueindex(1)));

   cAutoIPtr<IScriptable> pInstance = NULL;
   if (pFactory->CreateInstance((void * *)&pInstance) != S_OK || pInstance == NULL)
   {
      DebugMsg("ERROR: CreateInstance failed for lua class\n");
      return 0;
   }

   return LuaPublishObject(L, pInstance);
}

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

static int LuaThunkFunction(lua_State * L)
{
   tScriptFn pfn = (tScriptFn)lua_touserdata(L, lua_upvalueindex(1));
   if (pfn == NULL)
      return 0; // no C function to call

   int nArgs = lua_gettop(L);

   if (nArgs > kMaxArgs)
      nArgs = kMaxArgs;

   cScriptVar results[kMaxResults];
   int result = -1;

   if (nArgs > 0)
   {
      cScriptVar args[kMaxArgs];

      for (int i = 0; i < nArgs; i++)
      {
         switch (lua_type(L, i + 1))
         {
            case LUA_TNUMBER:
            {
               args[i] = lua_tonumber(L, i + 1);
               break;
            }

            case LUA_TSTRING:
            {
               args[i] = const_cast<char *>(lua_tostring(L, i + 1));
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

      lua_pop(L, nArgs);

      result = (*pfn)(nArgs, args, kMaxResults, results);
   }
   else
   {
      result = (*pfn)(0, NULL, kMaxResults, results);
   }

   if (result <= 0)
      return 0; // we are not returning any values on the stack

   for (int i = 0; i < result; i++)
   {
      switch (results[i].type)
      {
         case kNumber:
         {
            lua_pushnumber(L, results[i]);
            break;
         }

         case kString:
         {
            lua_pushstring(L, results[i]);
            break;
         }
      }
   }

   // on success, result is the number of return values
   return result;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLuaInterpreter
//

///////////////////////////////////////

void ScriptInterpreterCreate()
{
   cAutoIPtr<IScriptInterpreter> p(new cLuaInterpreter);
}

///////////////////////////////////////

cLuaInterpreter::cLuaInterpreter()
 : m_L(NULL)
{
}

///////////////////////////////////////

cLuaInterpreter::~cLuaInterpreter()
{
   DebugMsgIf(m_L != NULL, "WARNING: script machine destructor called before Term!\n");
}

///////////////////////////////////////

tResult cLuaInterpreter::Init()
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

tResult cLuaInterpreter::Term()
{
   if (m_L != NULL)
      lua_close(m_L);
   m_L = NULL;
   return S_OK;
}

///////////////////////////////////////

tResult cLuaInterpreter::ExecFile(const char * pszFile)
{
   if (m_L != NULL
      && lua_dofile(m_L, pszFile) == 0)
   {
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cLuaInterpreter::ExecString(const char * pszCode)
{
   if (m_L != NULL
      && lua_dobuffer(m_L, pszCode, strlen(pszCode), pszCode) == 0)
   {
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

void cLuaInterpreter::CallFunction(const char * pszName, const char * pszArgDesc, ...)
{
   Assert(pszName != NULL);
   va_list args;
   va_start(args, pszArgDesc);
   CallFunction(pszName, pszArgDesc, args);
   va_end(args);
}

///////////////////////////////////////

void cLuaInterpreter::CallFunction(const char * pszName, const char * pszArgDesc, va_list args)
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

tResult cLuaInterpreter::AddFunction(const char * pszName, tScriptFn pfn)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushlightuserdata(m_L, pfn);
      lua_pushcclosure(m_L, LuaThunkFunction, 1);
      lua_setglobal(m_L, pszName);
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cLuaInterpreter::RemoveFunction(const char * pszName)
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

tResult cLuaInterpreter::GetGlobal(const char * pszName, cScriptVar * pValue)
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

tResult cLuaInterpreter::GetGlobal(const char * pszName, double * pValue)
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
   return S_FALSE;
}

///////////////////////////////////////

tResult cLuaInterpreter::GetGlobal(const char * pszName, char * pValue, int cbMaxValue)
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
   return S_FALSE;
}

///////////////////////////////////////

void cLuaInterpreter::SetGlobal(const char * pszName, double value)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushnumber(m_L, value);
      lua_setglobal(m_L, pszName);
   }
}

///////////////////////////////////////

void cLuaInterpreter::SetGlobal(const char * pszName, const char * pszValue)
{
   Assert(pszName != NULL);
   if (m_L != NULL)
   {
      lua_pushstring(m_L, pszValue);
      lua_setglobal(m_L, pszName);
   }
}

///////////////////////////////////////

tResult cLuaInterpreter::RegisterCustomClass(const tChar * pszClassName,
                                             IScriptableFactory * pFactory)
{
   if (pszClassName == NULL && pFactory == NULL)
      return E_INVALIDARG;

   if (m_L == NULL)
      return E_FAIL;

   pFactory->AddRef();
   lua_pushlightuserdata(m_L, pFactory);
   lua_pushcclosure(m_L, LuaConstructObject, 1);
   lua_setglobal(m_L, pszClassName);

   return S_OK;
}

///////////////////////////////////////

tResult cLuaInterpreter::RevokeCustomClass(const tChar * pszClassName)
{
   if (pszClassName == NULL)
      return E_INVALIDARG;

   if (m_L == NULL)
      return E_FAIL;

   lua_getglobal(m_L, pszClassName);
   lua_getupvalue(m_L, -1, 1);
   IUnknown * pUnkFactory = static_cast<IUnknown *>(lua_touserdata(m_L, -1));
   lua_pop(m_L, 2); // pop the function and the up-value (getglobal and getupvalue results)

   lua_pushnil(m_L);
   lua_setglobal(m_L, pszClassName);

   Assert(pUnkFactory != NULL);
   SafeRelease(pUnkFactory);

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

double g_foo;

class cFoo : public cComObject<IMPLEMENTS(IScriptable)>
{
public:
   virtual tResult Invoke(const char * pszMethodName,
                          int nArgs, const cScriptVar * pArgs,
                          int nMaxResults, cScriptVar * pResults)
   {
      if (strcmp(pszMethodName, "SetFoo") == 0
         && nArgs == 1
         && pArgs[0].type == kNumber
         && nMaxResults > 0)
      {
         g_foo = pArgs[0].d;
         pResults[0] = g_foo;
         return 1;
      }
      return E_FAIL;
   }
};

class cFooFactory : public cComObject<IMPLEMENTS(IScriptableFactory)>
{
public:
   virtual tResult CreateInstance(void * * ppvInstance)
   {
      *ppvInstance = (void *)new cFoo;
      return S_OK;
   }
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRNG
//

class cRNG : public cComObject<IMPLEMENTS(IScriptable)>
{
public:
   static const char LuaClassName[];

   virtual tResult Invoke(const char * pszMethodName,
                          int nArgs, const cScriptVar * pArgs,
                          int nMaxResults, cScriptVar * pResults);

private:
   uint m_a;
   static const uint gm_b;
};

///////////////////////////////////////

const char cRNG::LuaClassName[] = "RNG";

const uint cRNG::gm_b = 523786821;

///////////////////////////////////////

tResult cRNG::Invoke(const char * pszMethodName,
                     int nArgs, const cScriptVar * pArgs,
                     int nMaxResults, cScriptVar * pResults)
{
   if (strcmp(pszMethodName, "Seed") == 0
      && nArgs == 1)
   {
      m_a = pArgs[0].d;
      return 0;
   }
   else if (strcmp(pszMethodName, "Rand") == 0
      && nArgs == 0
      && nMaxResults > 0)
   {
      m_a = m_a * gm_b + 1;
      pResults[0] = m_a;
      return 1;
   }
   return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRNGFactory
//

class cRNGFactory : public cComObject<IMPLEMENTS(IScriptableFactory)>
{
public:
   virtual tResult CreateInstance(void * * ppvInstance);
};

///////////////////////////////////////

tResult cRNGFactory::CreateInstance(void * * ppvInstance)
{
   *ppvInstance = (void *)new cRNG;
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLuaInterpreterTests
//

class cLuaInterpreterTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cLuaInterpreterTests);
      CPPUNIT_TEST(TestCallFunction);
      CPPUNIT_TEST(TestRemoveFunction);
      CPPUNIT_TEST(TestGetNumber);
      CPPUNIT_TEST(TestGetString);
      CPPUNIT_TEST(TestCustomClass);
      CPPUNIT_TEST(TestCustomClass2);
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
      CPPUNIT_ASSERT(m_pInterp->AddFunction("CallThisFunction", CallThisFunction) == S_OK);
      gm_bCalled = false;
      CPPUNIT_ASSERT(m_pInterp->ExecString("CallThisFunction();") == S_OK);
      CPPUNIT_ASSERT(gm_bCalled);
      CPPUNIT_ASSERT(m_pInterp->RemoveFunction("CallThisFunction") == S_OK);
   }

   void TestRemoveFunction()
   {
      CPPUNIT_ASSERT(m_pInterp->AddFunction("RemoveThisFunction", RemoveThisFunction) == S_OK);
      CPPUNIT_ASSERT(m_pInterp->ExecString("RemoveThisFunction();") == S_OK);
      CPPUNIT_ASSERT(m_pInterp->RemoveFunction("RemoveThisFunction") == S_OK);
      CPPUNIT_ASSERT(m_pInterp->ExecString("RemoveThisFunction();") != S_OK);

      CPPUNIT_ASSERT(m_pInterp->ExecString("ThisNameWillNotBeFoundSoThisCallShouldFail();") != S_OK);
   }

   void TestGetNumber()
   {
      double value;

      m_pInterp->SetGlobal("foo", 123.456);
      CPPUNIT_ASSERT(m_pInterp->GetGlobal("foo", &value) == S_OK);
      CPPUNIT_ASSERT(value == 123.456);
   }

   void TestGetString()
   {
      char szValue[16];

      m_pInterp->SetGlobal("bar", "blah blah");
      CPPUNIT_ASSERT(m_pInterp->GetGlobal("bar", szValue, _countof(szValue)) == S_OK);
      CPPUNIT_ASSERT(strcmp(szValue, "blah blah") == 0);

      m_pInterp->SetGlobal("bar", "blah blah blah blah blah blah blah blah");
      CPPUNIT_ASSERT(m_pInterp->GetGlobal("bar", szValue, _countof(szValue)) == S_OK);
      CPPUNIT_ASSERT(strcmp(szValue, "blah blah blah ") == 0);
   }

   void TestCustomClass()
   {
      cAutoIPtr<IScriptableFactory> pFooFactory = new cFooFactory;

      CPPUNIT_ASSERT(m_pInterp->RegisterCustomClass("foo", pFooFactory) == S_OK);

      CPPUNIT_ASSERT(m_pInterp->ExecString("f = foo(); f:SetFoo(1000);") == S_OK);
      CPPUNIT_ASSERT(g_foo == 1000);

      CPPUNIT_ASSERT(m_pInterp->ExecString("f = foo(); f:SetFoo(3.1415);") == S_OK);
      CPPUNIT_ASSERT(g_foo == 3.1415);

      CPPUNIT_ASSERT(m_pInterp->RevokeCustomClass("foo") == S_OK);

      CPPUNIT_ASSERT(m_pInterp->ExecString("f = foo(); f:SetFoo(99);") != S_OK);
   }

   void TestCustomClass2()
   {
      cAutoIPtr<IScriptableFactory> pRNGFactory = new cRNGFactory;

      CPPUNIT_ASSERT(m_pInterp->RegisterCustomClass(cRNG::LuaClassName, pRNGFactory) == S_OK);

      static const char scriptSpec[] =
      {
         "rng = RNG();" \
         "rng:Seed(%d);" \
         "local r = rng:Rand();" \
         "print([[Rand() returned ]] .. r .. [[\r\n]]);"
      };

      char szScript[1024];
      sprintf(szScript, scriptSpec, time(NULL));

      CPPUNIT_ASSERT(m_pInterp->ExecString(szScript) == S_OK);

      CPPUNIT_ASSERT(m_pInterp->RevokeCustomClass(cRNG::LuaClassName) == S_OK);

      CPPUNIT_ASSERT(m_pInterp->ExecString(szScript) != S_OK);
   }

   cAutoIPtr<IScriptInterpreter> m_pInterp;

public:
   virtual void setUp()
   {
      CPPUNIT_ASSERT(!m_pInterp);
      m_pInterp = (IScriptInterpreter *)FindGlobalObject(IID_IScriptInterpreter);
   }

   virtual void tearDown()
   {
      SafeRelease(m_pInterp);
   }
};

bool cLuaInterpreterTests::gm_bCalled = false;

CPPUNIT_TEST_SUITE_REGISTRATION(cLuaInterpreterTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
