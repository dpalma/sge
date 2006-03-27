///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "luainterp.h"
#include "luautil.h"

#include "multivar.h"
#include "techstring.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

extern "C"
{
#include <lualib.h>
#include <lauxlib.h>
}

#include <cstdio>
#include <ctime>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#ifdef STATIC_BUILD
extern sScriptReg cmds[];
void * pReferenceCmds = (void *)cmds;
#endif

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(LuaInterp);

#define LocalMsg(msg)                  DebugMsgEx(LuaInterp,msg)
#define LocalMsg1(msg,a1)              DebugMsgEx1(LuaInterp,msg,(a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(LuaInterp,msg,(a1),(a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(LuaInterp,(cond),msg)
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(LuaInterp,(cond),msg,(a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(LuaInterp,(cond),msg,(a1),(a2))

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
         techlog.Print(kInfo, "\t");
      techlog.Print(kInfo, s);
#endif

      lua_pop(L, 1);  /* pop result */
   }
   fputs("\n", stdout);
   return 0;
}

///////////////////////////////////////////////////////////////////////////////
// modified version of Lua _ALERT function

int LuaAlertEx(lua_State * L)
{
   int nArgs = lua_gettop(L);
   if (nArgs == 1 && lua_isstring(L, -1))
   {
      const char * psz = lua_tostring(L, -1);
      techlog.Print(kError, "LUA: %s\n", psz);
      lua_pop(L, 1);
   }
   return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLuaInterpreter
//

///////////////////////////////////////

tResult ScriptInterpreterCreate()
{
   cAutoIPtr<cLuaInterpreter> pLuaInterpreter(new cLuaInterpreter);
   if (!pLuaInterpreter)
   {
      return E_OUTOFMEMORY;
   }

   pLuaInterpreter->m_bRegisterPreRegisteredFunctions = true;

   return RegisterGlobalObject(IID_IScriptInterpreter, static_cast<IScriptInterpreter*>(pLuaInterpreter));
}

///////////////////////////////////////

tResult ScriptAddFunction(const char * pszName, tScriptFn pfn)
{
   if (pszName == NULL || pfn == NULL)
   {
      return E_POINTER;
   }

   if (cLuaInterpreter::gm_bInitialized)
   {
      UseGlobal(ScriptInterpreter);
      return pScriptInterpreter->AddNamedItem(pszName, pfn);
   }
   else
   {
      // simple queue to support adding at static init time
      cLuaInterpreter::sPreRegisteredFunction * p = new cLuaInterpreter::sPreRegisteredFunction;
      if (p != NULL)
      {
         strcpy(p->szName, pszName);
         p->pfn = pfn;
         p->pNext = cLuaInterpreter::gm_pPreRegisteredFunctions;
         cLuaInterpreter::gm_pPreRegisteredFunctions = p;
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult ScriptAddFunctions(const sScriptReg * pFns, uint nFns)
{
   if (pFns == NULL)
   {
      return E_POINTER;
   }

   for (uint i = 0; i < nFns; i++, pFns++)
   {
      tResult result = ScriptAddFunction(pFns->pszName, pFns->pfn);
      if (FAILED(result))
      {
         return result;
      }
   }

   return S_OK;
}

///////////////////////////////////////

cScriptAutoAddFunction::cScriptAutoAddFunction(const char * pszName, tScriptFn pfn)
{
   ScriptAddFunction(pszName, pfn);
}

///////////////////////////////////////

bool cLuaInterpreter::gm_bInitialized = false;

///////////////////////////////////////

struct cLuaInterpreter::sPreRegisteredFunction * cLuaInterpreter::gm_pPreRegisteredFunctions = NULL;

///////////////////////////////////////

cLuaInterpreter::cLuaInterpreter()
 : m_bRegisterPreRegisteredFunctions(false)
{
}

///////////////////////////////////////

cLuaInterpreter::~cLuaInterpreter()
{
}

///////////////////////////////////////

tResult cLuaInterpreter::Init()
{
   if (!m_luaState.Open())
   {
      return E_FAIL;
   }

   lua_register(m_luaState.AccessLuaState(), "print", LuaPrintEx);
   lua_register(m_luaState.AccessLuaState(), "_ALERT", LuaAlertEx);

   if (m_bRegisterPreRegisteredFunctions)
   {
      Assert(!cLuaInterpreter::gm_bInitialized);
      gm_bInitialized = true;

      RegisterPreRegisteredFunctions();
   }

   return S_OK;
}

///////////////////////////////////////

tResult cLuaInterpreter::Term()
{
   m_luaState.Close();
   return S_OK;
}

///////////////////////////////////////

tResult cLuaInterpreter::ExecFile(const tChar * pszFile)
{
   return m_luaState.DoFile(pszFile);
}

///////////////////////////////////////

tResult cLuaInterpreter::ExecString(const char * pszCode)
{
   return m_luaState.DoString(pszCode);
}

///////////////////////////////////////

tResult CDECL cLuaInterpreter::CallFunction(const char * pszName, const char * pszArgDesc, ...)
{
   if (pszName == NULL)
   {
      return E_POINTER;
   }
   va_list args;
   va_start(args, pszArgDesc);
   tResult result = m_luaState.CallFunction(pszName, pszArgDesc, args);
   va_end(args);
   return result;
}

///////////////////////////////////////

tResult cLuaInterpreter::RegisterCustomClass(const tChar * pszClassName,
                                             IScriptableFactory * pFactory)
{
   if (pszClassName == NULL && pFactory == NULL)
   {
      return E_POINTER;
   }

   if (m_luaState.AccessLuaState() == NULL)
   {
      return E_FAIL;
   }

   pFactory->AddRef();
   lua_pushlightuserdata(m_luaState.AccessLuaState(), pFactory);
   lua_pushcclosure(m_luaState.AccessLuaState(), LuaConstructObject, 1);
   lua_setglobal(m_luaState.AccessLuaState(), pszClassName);

   return S_OK;
}

///////////////////////////////////////

tResult cLuaInterpreter::RevokeCustomClass(const tChar * pszClassName)
{
   if (pszClassName == NULL)
   {
      return E_POINTER;
   }

   lua_State * L = m_luaState.AccessLuaState();

   if (L == NULL)
   {
      return E_FAIL;
   }

   lua_getglobal(L, pszClassName);
   lua_getupvalue(L, -1, 1);
   IUnknown * pUnkFactory = static_cast<IUnknown *>(lua_touserdata(L, -1));
   lua_pop(L, 2); // pop the function and the up-value (getglobal and getupvalue results)

   lua_pushnil(L);
   lua_setglobal(L, pszClassName);

   Assert(pUnkFactory != NULL);
   SafeRelease(pUnkFactory);

   return S_OK;
}

///////////////////////////////////////

tResult cLuaInterpreter::AddNamedItem(const char * pszName, double value)
{
   return m_luaState.SetGlobal(pszName, value);
}

///////////////////////////////////////

tResult cLuaInterpreter::AddNamedItem(const char * pszName, const char * pszValue)
{
   return m_luaState.SetGlobal(pszName, pszValue);
}

///////////////////////////////////////

tResult cLuaInterpreter::AddNamedItem(const char * pszName, tScriptFn pfn)
{
   return m_luaState.SetGlobal(pszName, pfn);
}

///////////////////////////////////////

tResult cLuaInterpreter::AddNamedItem(const char * pszName, IScriptable * pObject)
{
   if (pszName == NULL || pObject == NULL)
   {
      return E_POINTER;
   }
   if (m_luaState.AccessLuaState() == NULL)
   {
      return E_FAIL;
   }
   // LuaPublishObject AddRefs the instance pointer so don't do so here
   if (LuaPublishObject(m_luaState.AccessLuaState(), pObject) != 1)
   {
      return E_FAIL;
   }
   lua_setglobal(m_luaState.AccessLuaState(), pszName);
   return S_OK;
}

///////////////////////////////////////

tResult cLuaInterpreter::RemoveNamedItem(const char * pszName)
{
   return m_luaState.RemoveGlobal(pszName);
}

///////////////////////////////////////

tResult cLuaInterpreter::GetNamedItem(const char * pszName, tScriptVar * pValue) const
{
   return m_luaState.GetGlobal(pszName, pValue);
}

///////////////////////////////////////

tResult cLuaInterpreter::GetNamedItem(const char * pszName, double * pValue) const
{
   return m_luaState.GetGlobal(pszName, pValue);
}

///////////////////////////////////////

tResult cLuaInterpreter::GetNamedItem(const char * pszName, char * pValue, int cbMaxValue) const
{
   return m_luaState.GetGlobal(pszName, pValue, cbMaxValue);
}

///////////////////////////////////////

void cLuaInterpreter::RegisterPreRegisteredFunctions()
{
   while (gm_pPreRegisteredFunctions != NULL)
   {
      AddNamedItem(gm_pPreRegisteredFunctions->szName, gm_pPreRegisteredFunctions->pfn);
      sPreRegisteredFunction * p = gm_pPreRegisteredFunctions;
      gm_pPreRegisteredFunctions = gm_pPreRegisteredFunctions->pNext;
      delete p;
   }
}

///////////////////////////////////////

void cLuaInterpreter::CleanupPreRegisteredFunctions()
{
   sPreRegisteredFunction * p = gm_pPreRegisteredFunctions;
   while (p != NULL)
   {
      gm_pPreRegisteredFunctions = p->pNext;
      delete p;
      p = gm_pPreRegisteredFunctions;
   }
}

///////////////////////////////////////

cLuaInterpreter::cAutoCleanupPreRegisteredFunctions::~cAutoCleanupPreRegisteredFunctions()
{
   CleanupPreRegisteredFunctions();
}

///////////////////////////////////////

cLuaInterpreter::cAutoCleanupPreRegisteredFunctions cLuaInterpreter::g_autoCleanupPreRegisteredFunctions;


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

double g_foo;

class cFooScriptable : public cComObject<IMPLEMENTS(IScriptable)>
{
public:
   virtual tResult Invoke(const char * pszMethodName,
                          int nArgs, const tScriptVar * pArgs,
                          int nMaxResults, tScriptVar * pResults)
   {
      if (strcmp(pszMethodName, "SetFoo") == 0
         && nArgs == 1
         && (pArgs[0].IsInt() || pArgs[0].IsFloat() || pArgs[0].IsDouble())
         && nMaxResults > 0)
      {
         g_foo = pArgs[0].ToDouble();
         pResults[0] = g_foo;
         return 1;
      }
      return E_FAIL;
   }
};

class cFooScriptableFactory : public cComObject<IMPLEMENTS(IScriptableFactory)>
{
public:
   virtual tResult CreateInstance(void * * ppvInstance)
   {
      *ppvInstance = (void *)new cFooScriptable;
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
                          int nArgs, const tScriptVar * pArgs,
                          int nMaxResults, tScriptVar * pResults);

private:
   int m_a;
   static const int gm_b;
};

///////////////////////////////////////

const char cRNG::LuaClassName[] = "RNG";

const int cRNG::gm_b = 523786821;

///////////////////////////////////////

tResult cRNG::Invoke(const char * pszMethodName,
                     int nArgs, const tScriptVar * pArgs,
                     int nMaxResults, tScriptVar * pResults)
{
   if (strcmp(pszMethodName, "Seed") == 0
      && nArgs == 1)
   {
      m_a = pArgs[0];
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

class cLuaInterpreterTests
{
protected:
   cLuaInterpreterTests();
   ~cLuaInterpreterTests();

   cAutoIPtr<IScriptInterpreter> m_pInterp;
};

////////////////////////////////////////

cLuaInterpreterTests::cLuaInterpreterTests()
{
   m_pInterp = static_cast<IScriptInterpreter *>(new cLuaInterpreter);
   cAutoIPtr<IGlobalObject> pGO;
   if (m_pInterp->QueryInterface(IID_IGlobalObject, (void**)&pGO) == S_OK)
   {
      pGO->Init();
   }
}

////////////////////////////////////////

cLuaInterpreterTests::~cLuaInterpreterTests()
{
   cAutoIPtr<IGlobalObject> pGO;
   if (m_pInterp->QueryInterface(IID_IGlobalObject, (void**)&pGO) == S_OK)
   {
      pGO->Term();
   }
   SafeRelease(m_pInterp);
}

////////////////////////////////////////

TEST_F(cLuaInterpreterTests, CustomClass)
{
   cAutoIPtr<IScriptableFactory> pFooFactory = new cFooScriptableFactory;

   CHECK(m_pInterp->RegisterCustomClass("foo", pFooFactory) == S_OK);

   CHECK(m_pInterp->ExecString("f = foo(); f:SetFoo(1000);") == S_OK);
   CHECK_EQUAL(g_foo, 1000);

   CHECK(m_pInterp->ExecString("f = foo(); f:SetFoo(3.1415);") == S_OK);
   CHECK_EQUAL(g_foo, 3.1415);

   CHECK(m_pInterp->RevokeCustomClass("foo") == S_OK);

   CHECK(m_pInterp->ExecString("f = foo(); f:SetFoo(99);") != S_OK);
}

////////////////////////////////////////

TEST_F(cLuaInterpreterTests, CustomClass2)
{
   cAutoIPtr<IScriptableFactory> pRNGFactory = new cRNGFactory;

   CHECK(m_pInterp->RegisterCustomClass(cRNG::LuaClassName, pRNGFactory) == S_OK);

   static const char scriptSpec[] =
   {
      "rng = RNG();" \
      "rng:Seed(%d);" \
      "local r = rng:Rand();" \
      "print([[Rand() returned ]] .. r .. [[\r\n]]);"
   };

   cStr script;
   Sprintf(&script, scriptSpec, time(NULL));

   CHECK(m_pInterp->ExecString(script.c_str()) == S_OK);
   CHECK(m_pInterp->RevokeCustomClass(cRNG::LuaClassName) == S_OK);
   CHECK(m_pInterp->ExecString(script.c_str()) != S_OK);
}

////////////////////////////////////////

TEST_F(cLuaInterpreterTests, PublishObject)
{
   cAutoIPtr<IScriptable> pRNG = new cRNG;
   CHECK(!!pRNG);

   static const char kRNG[] = "rngObject";
   CHECK(m_pInterp->AddNamedItem(kRNG, pRNG) == S_OK);

   static const char kScriptSpec[] =
   {
      "%s:Seed(%d);" \
      "local r = %s:Rand();" \
      "print([[%s() returned ]] .. r .. [[\r\n]]);"
   };

   int randSeed = time(NULL);

   cStr script;
   Sprintf(&script, kScriptSpec, kRNG, randSeed, kRNG, kRNG);

   CHECK(m_pInterp->ExecString(script.c_str()) == S_OK);
   CHECK(m_pInterp->RemoveNamedItem(kRNG) == S_OK);
   CHECK(m_pInterp->ExecString(script.c_str()) != S_OK);
}

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
