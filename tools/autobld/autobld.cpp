// $Id$

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

#ifndef EXTERN_C
#define EXTERN_C extern "C"
#endif

#ifndef DECLSPEC_DLLEXPORT
#define DECLSPEC_DLLEXPORT __declspec(dllexport)
#endif



static int autobld_GetRegistryValue(lua_State *L)
{
   int result = 0;

   HKEY hBaseKey = NULL;
   const char * pszKey = NULL;
   const char * pszValue = NULL;

   if (lua_gettop(L) > 3)
   {
      luaL_checknumber(L, -3);
      luaL_checkstring(L, -2);
      luaL_checkstring(L, -1);
      hBaseKey = (HKEY)(long)lua_tonumber(L, -3);
      pszKey = lua_tostring(L, -2);
      pszValue =  lua_tostring(L, -1);
   }
   else
   {
      luaL_checknumber(L, -2);
      luaL_checkstring(L, -1);
      hBaseKey = (HKEY)(long)lua_tonumber(L, -2);
      pszKey = lua_tostring(L, -1);
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


static void PushError(lua_State *L, DWORD dwError)
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


static int autobld_Mkdir(lua_State *L)
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



static int autobld_Cwd(lua_State *L)
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


static const luaL_reg autobld_funcs[] =
{
   { "getregistryvalue", autobld_GetRegistryValue },
   { "dir", autobld_Dir },
   { "mkdir", autobld_Mkdir },
   { "cwd", autobld_Cwd },
   { "setenv", autobld_Setenv },
   { NULL, NULL }
};


#define LUA_AUTOBLDLIBNAME "autobld"
EXTERN_C DECLSPEC_DLLEXPORT int init(lua_State *L)
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

