// $Id$

#pragma warning(disable:4786)

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <malloc.h>
#ifdef _DEBUG
#define DEBUG
#include <dprintf.h>
#else
inline void dprintf(...) {}
#endif

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <map>
#include <string>

#ifndef DECLSPEC_DLLEXPORT
#define DECLSPEC_DLLEXPORT __declspec(dllexport)
#endif


// NOTE: the first argument (at index 1) to all of these functions is a
// table that represents all the functions in this library.


static int autobld_GetRegistryValue(lua_State *L)
{
   int result = 0;

   int top = lua_gettop(L);
   if (top < 3)
   {
      lua_pushliteral(L, "Invalid number of arguments to getregistryvalue");
      lua_error(L);
   }

   // See note above about what's at index 1
   luaL_checknumber(L, 2);
   HKEY hBaseKey = (HKEY)(long)lua_tonumber(L, 2);
   luaL_checkstring(L, 3);
   const char * pszKey = lua_tostring(L, 3);
   const char * pszValue = NULL;

   if (lua_gettop(L) > 3)
   {
      luaL_checkstring(L, -1);
      pszValue =  lua_tostring(L, -1);
   }

   if (hBaseKey == HKEY_LOCAL_MACHINE || hBaseKey == HKEY_CURRENT_USER)
   {
      HKEY hKey = NULL;
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
      {
         DWORD dwType, dwSize;
         if (RegQueryValueEx(hKey, pszValue, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS)
         {
            if (dwType == REG_SZ)
            {
               char * pszData = (char *)_alloca(dwSize + 1);
               if (pszData != NULL)
               {
                  if (RegQueryValueEx(hKey, pszValue, NULL, &dwType, (LPBYTE)pszData, &dwSize) == ERROR_SUCCESS)
                  {
                     lua_pushstring(L, pszData);
                     result = 1;
                  }
               }
            }
            else if (dwType == REG_DWORD)
            {
               DWORD dwData;
               if (RegQueryValueEx(hKey, pszValue, NULL, &dwType, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
               {
                  lua_pushnumber(L, dwData);
                  result = 1;
               }
            }
         }
         RegCloseKey(hKey);
      }
   }

   return result;
}


static int autobld_Dir(lua_State *L)
{
   luaL_checkstring(L, -1);
   WIN32_FIND_DATA findData = {0};
   const char * pszSpec = lua_tostring(L, -1);
   HANDLE hFinder = FindFirstFile(pszSpec, &findData);
   if (hFinder != NULL && hFinder != INVALID_HANDLE_VALUE)
   {
      int index = 1;
      lua_newtable(L);

      do
      {
         if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            if (lstrcmp(findData.cFileName, ".") == 0 ||
                lstrcmp(findData.cFileName, "..") == 0)
            {
               continue;
            }
         }

         lua_pushnumber(L, index++);
         lua_pushstring(L, findData.cFileName);
         lua_settable(L, -3);
      }
      while (FindNextFile(hFinder, &findData));

      FindClose(hFinder);

      return 1;
   }

   return 0;
}


static void PushError(lua_State * L, DWORD dwError)
{
   char buffer[128];
   if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      0, dwError, 0, buffer, sizeof(buffer), 0))
   {
      lua_pushstring(L, buffer);
   }
   else
   {
      lua_pushfstring(L, "system error %d\n", dwError);
   }
}


static int autobld_Mkdir(lua_State * L)
{
   const char * pszDir = NULL;
   bool bFailOnExisting = false;
   if (lua_gettop(L) > 2)
   {
      if (!lua_isboolean(L, -1))
      {
         char szMsg[200];
         wsprintf(szMsg, "boolean expected, got %s", lua_typename(L, -1));
         lua_pushstring(L, szMsg);
         lua_error(L);
      }
      luaL_checkstring(L, -2);
      bFailOnExisting = lua_toboolean(L, -1) ? true : false;
      pszDir = lua_tostring(L, -2);
   }
   else
   {
      luaL_checkstring(L, -1);
      pszDir = lua_tostring(L, -1);
   }
   if (!CreateDirectory(pszDir, NULL))
   {
      DWORD dwError = GetLastError();
      if (dwError != ERROR_ALREADY_EXISTS || bFailOnExisting)
      {
         PushError(L, dwError);
         lua_error(L);
      }
   }
   return 0;
}



static int autobld_Cwd(lua_State * L)
{
   char szCwd[MAX_PATH];
   if (!GetCurrentDirectory(sizeof(szCwd) / sizeof(szCwd[0]), szCwd))
   {
      PushError(L, GetLastError());
      lua_error(L);
   }
   lua_pushstring(L, szCwd);
   return 1;
}



static int autobld_Setenv(lua_State * L)
{
   const char * pszValue = NULL;
   if (lua_isstring(L, -1))
   {
      luaL_checkstring(L, -1);
      pszValue = lua_tostring(L, -1);
   }
   luaL_checkstring(L, -2);
   const char * pszName = lua_tostring(L, -2);
   if (!SetEnvironmentVariable(pszName, pszValue))
   {
      PushError(L, GetLastError());
      lua_error(L);
   }
   return 0;
}



static int autobld_Spawn(lua_State * L)
{
   int top = lua_gettop(L);
   if (top < 2)
   {
      lua_pushliteral(L, "Invalid number of arguments to spawn");
      lua_error(L);
   }

   // See note above about what's at index 1

   luaL_checkstring(L, 2);
   const char * pszCmd = lua_tostring(L, 2);

   typedef std::map<std::string, std::string> tEnv;
   tEnv env;

   if (lua_istable(L, 3))
   {
      lua_pushnil(L);
      while (lua_next(L, 3))
      {
         const char * pszKey = lua_tostring(L, -2);
         const char * pszVal = lua_tostring(L, -1);
         if (pszKey != NULL && pszVal != NULL)
         {
            env.insert(std::make_pair(pszKey, pszVal));
         }
         lua_pop(L, 1);
      }
   }

   char * pEnv = NULL;
   if (!env.empty())
   {
      unsigned envSize = 1;

      tEnv::iterator iter = env.begin();
      tEnv::iterator end = env.end();
      for (; iter != end; iter++)
      {
         envSize += iter->first.length();
         envSize += iter->second.length();
         envSize += 2; // for equal sign and null terminator
      }

      pEnv = new char[envSize];
      if (pEnv == NULL)
      {
         lua_pushliteral(L, "Error allocating environment");
         lua_error(L);
      }

      char * p = pEnv;

      iter = env.begin();
      end = env.end();
      for (; iter != end; iter++)
      {
         unsigned l = iter->first.length() + iter->second.length() + 1;
         wsprintf(p, "%s=%s", iter->first.c_str(), iter->second.c_str());
         p += (l + 1);
      }

      *p = 0;
   }

   STARTUPINFO si = {0};
   PROCESS_INFORMATION pi = {0};
   BOOL bResult = CreateProcess(NULL, const_cast<char*>(pszCmd), NULL, NULL, FALSE, 0, pEnv, NULL, &si, &pi);

   delete [] pEnv;
   pEnv = NULL;

   if (!bResult)
   {
      PushError(L, GetLastError());
      lua_error(L);
   }

   CloseHandle(pi.hThread);
   CloseHandle(pi.hProcess);

   lua_pushnumber(L, pi.dwProcessId);
   return 1;
}



static const luaL_reg autobld_funcs[] =
{
   { "getregistryvalue", autobld_GetRegistryValue },
   { "dir", autobld_Dir },
   { "mkdir", autobld_Mkdir },
   { "cwd", autobld_Cwd },
   { "setenv", autobld_Setenv },
   { "spawn", autobld_Spawn },
   { NULL, NULL }
};


#define LUA_AUTOBLDLIBNAME "autobld"
extern "C" DECLSPEC_DLLEXPORT int init(lua_State *L)
{
   luaL_openlib(L, LUA_AUTOBLDLIBNAME, autobld_funcs, 0);

   lua_pushliteral(L, "HKEY_LOCAL_MACHINE");
   lua_pushnumber(L, (double)(long)HKEY_LOCAL_MACHINE);
   lua_settable(L, LUA_GLOBALSINDEX);

   lua_pushliteral(L, "HKEY_CURRENT_USER");
   lua_pushnumber(L, (double)(long)HKEY_CURRENT_USER);
   lua_settable(L, LUA_GLOBALSINDEX);

   return 0;
}



BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      case DLL_PROCESS_DETACH:
         break;
   }
   return TRUE;
}

