///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "luautil.h"
#include "scriptapi.h"

#include "dictionaryapi.h"
#include "multivar.h"

extern "C"
{
#include <lualib.h>
#include <lauxlib.h>
}

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(LuaUtil);

#define LocalMsg(msg)                  DebugMsgEx(LuaUtil,msg)
#define LocalMsg1(msg,a1)              DebugMsgEx1(LuaUtil,msg,(a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(LuaUtil,msg,(a1),(a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(LuaUtil,(cond),msg)
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(LuaUtil,(cond),msg,(a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(LuaUtil,(cond),msg,(a1),(a2))


///////////////////////////////////////////////////////////////////////////////

void LuaGetArg(lua_State * L, int index, tScriptVar * pArg)
{
#ifndef NDEBUG
   const char * pszTypeName = lua_typename(L, lua_type(L, index));
   LocalMsg2("LuaGetArg: arg at index %d is type \"%s\"\n", index, pszTypeName);
#endif

   switch (lua_type(L, index))
   {
      case LUA_TNUMBER:
      {
         *pArg = lua_tonumber(L, index);
         break;
      }

      case LUA_TSTRING:
      {
         *pArg = const_cast<char *>(lua_tostring(L, index));
         break;
      }

      case LUA_TTABLE:
      {
         cAutoIPtr<IDictionary> pDict(DictionaryCreate());
         if (!!pDict)
         {
            int nDictEntries = 0;
            lua_pushnil(L);
            while (lua_next(L, index))
            {
               const char * pszVal = lua_tostring(L, -1);
               // It's best not to call lua_tostring for the key because
               // it changes the stack, potentially confusing lua_next.
               // For now, only string keys are supported because that's
               // more naturaly for IDictionary. Numeric keys mean that
               // an array was passed in.
               int keyType = lua_type(L, -2);
               switch (keyType)
               {
                  case LUA_TSTRING:
                  {
                     const char * pszKey = lua_tostring(L, -2);
                     pDict->Set(cMultiVar(pszKey).ToString(), pszVal);
                     nDictEntries++;
                     break;
                  }
                  case LUA_TNUMBER:
                  {
                     double key = lua_tonumber(L, -2);
                     break;
                  }
               }
               lua_pop(L, 1);
            }
            if (nDictEntries > 0)
            {
               *pArg = CTAddRef(pDict);
            }
         }
         else
         {
            // if failed to create dictionary object, indicate that
            // the script function should have received an interface
            // pointer
            pArg->Assign(static_cast<IUnknown*>(NULL));
         }
         break;
      }

      default:
      {
         pArg->Clear();
         break;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

int LuaPushResults(lua_State * L, int nResults, tScriptVar * results)
{
   int nResultsPushed = 0;

   for (int i = 0; i < nResults; i++)
   {
      switch (results[i].GetType())
      {
         case kMVT_Int:
         case kMVT_Float:
         case kMVT_Double:
         {
            lua_pushnumber(L, results[i].ToDouble());
            nResultsPushed++;
            break;
         }

         case kMVT_String:
         {
            lua_pushstring(L, results[i]);
            nResultsPushed++;
            break;
         }

         case kMVT_Interface:
         {
            cAutoIPtr<IDictionary> pDict;
            if (static_cast<IUnknown*>(results[i])->QueryInterface(IID_IDictionary, (void**)&pDict) == S_OK)
            {
               std::list<cStr> keys;
               if (pDict->GetKeys(&keys) == S_OK)
               {
                  lua_newtable(L);
                  std::list<cStr>::const_iterator iter = keys.begin();
#ifdef __GNUC__
                  if (strtol(keys.front().c_str(), NULL, 10) == 1)
#else
                  if (_ttoi(keys.front().c_str()) == 1)
#endif
                  {
                     for (int index = 1; iter != keys.end(); iter++, index++)
                     {
#ifdef __GNUC__
                        int iterAsInt = strtol(iter->c_str(), NULL, 10);
#else
                        int iterAsInt = _ttoi(iter->c_str());
#endif
                        WarnMsgIf2(iterAsInt != index, "Expected numeric key %d, got \"%s\"", index, iter->c_str());
                        cMultiVar value;
                        if (pDict->Get(iter->c_str(), &value) == S_OK)
                        {
                           lua_pushnumber(L, index);
                           lua_pushstring(L, value.ToAsciiString());
                           lua_settable(L, -3);
                        }
                     }
                  }
                  else
                  {
                     for (; iter != keys.end(); iter++)
                     {
                        cMultiVar value;
                        if (pDict->Get(iter->c_str(), &value) == S_OK)
                        {
                           cMultiVar key(iter->c_str());
                           lua_pushstring(L, key.ToAsciiString());
                           lua_pushstring(L, value.ToAsciiString());
                           lua_settable(L, -3);
                        }
                     }
                  }
                  nResultsPushed++;
               }
            }
            cAutoIPtr<IScriptable> pScriptable;
            if (static_cast<IUnknown*>(results[i])->QueryInterface(IID_IScriptable, (void**)&pScriptable) == S_OK)
            {
               nResultsPushed += LuaPublishObject(L, pScriptable);
            }
            break;
         }

         case kMVT_Empty:
         {
            lua_pushnil(L);
            nResultsPushed++;
            break;
         }
      }
   }

   return nResultsPushed;
}

///////////////////////////////////////////////////////////////////////////////

static int LuaThunkInvoke(lua_State * L)
{
   // the name of the method to call is at the top of the stack
   const char * pszMethodName = lua_tostring(L, lua_upvalueindex(1));

   int nArgsOnStack = lua_gettop(L);

   if (nArgsOnStack == 1 && lua_type(L, 1) != LUA_TUSERDATA)
   {
      char szMsg[200];
#if _MSC_VER >= 1400
      _snprintf_s(szMsg, sizeof(szMsg), _countof(szMsg),
#else
      _snprintf(szMsg, _countof(szMsg),
#endif
         "invalid method call: %s called with no instance pointer", pszMethodName);
      lua_pushstring(L, szMsg);
      lua_error(L); // this function never returns
   }

#if 0
   {
      DebugMsg1("LUA: call %s\n", pszMethodName);
      for (int i = 0, index = -1; i < nArgsOnStack; i++, index--)
      {
         int type = lua_type(L, index);
         DebugMsg2("LUA:   stack[%d] type %d\n", index, type);
      }
   }
#endif

   // the "this" pointer is at the bottom of the stack
   IScriptable * pInstance = static_cast<IScriptable *>(lua_unboxpointer(L, 1));
   // Subtract one to exclude the "this" pointer
   nArgsOnStack -= 1;

   int nArgs = Min(kLuaMaxArgs, nArgsOnStack);

   tScriptVar results[kLuaMaxResults];
   int result = -1;

   if (nArgs > 0)
   {
      tScriptVar args[kLuaMaxArgs];

      for (int i = 0; i < nArgs; i++)
      {
         LuaGetArg(L, i + 2, &args[i]);
      }

      lua_pop(L, nArgsOnStack);

      result = pInstance->Invoke(pszMethodName, nArgs, args, kLuaMaxResults, results);
   }
   else
   {
      result = pInstance->Invoke(pszMethodName, 0, NULL, kLuaMaxResults, results);
   }

   if (FAILED(result))
   {
      WarnMsg2("IScriptable[%p]->Invoke(%s, ...) failed\n", pInstance, pszMethodName);
      return 0;
   }

   Assert(result <= kLuaMaxResults);
   if (result > kLuaMaxResults)
   {
      result = kLuaMaxResults;
   }

   int nResultsPushed = LuaPushResults(L, result, results);
   lua_checkstack(L, nResultsPushed);
   return nResultsPushed;
}

///////////////////////////////////////////////////////////////////////////////

static int LuaGarbageCollectInterface(lua_State * L)
{
   IUnknown * pUnk = static_cast<IUnknown *>(lua_unboxpointer(L, 1));
   Assert(pUnk != NULL);
   SafeRelease(pUnk);
   return 0;
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

int LuaPublishObject(lua_State * L, IScriptable * pInstance)
{
   lua_boxpointer(L, CTAddRef(pInstance));

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
