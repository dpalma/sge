///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engineapi.h"
#include "entityapi.h"
#include "inputapi.h"
#include "scriptapi.h"
#include "sys.h"

#include "dictionaryapi.h"
#include "globalobj.h"
#include "keys.h"
#include "multivar.h"
#include "resourceapi.h"
#include "vec3.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

inline bool IsNumber(const tScriptVar & scriptVar)
{
   return scriptVar.IsInt() || scriptVar.IsFloat() || scriptVar.IsDouble();
}

#define ScriptArgIsString(iArg) (argv[iArg].IsString())
#define ScriptArgIsNumber(iArg) (IsNumber(argv[iArg]))

///////////////////////////////////////////////////////////////////////////////

static const struct
{
   const char * pszKeyName;
   uint keyCode;
}
g_keyNames[] =
{
   { "F1", kF1 },
   { "F2", kF2 },
   { "F3", kF3 },
   { "F4", kF4 },
   { "F5", kF5 },
   { "F6", kF6 },
   { "F7", kF7 },
   { "F8", kF8 },
   { "F9", kF9 },
   { "F10", kF10 },
   { "F11", kF11 },
   { "F12", kF12 },
   { "Insert", kInsert },
   { "Delete", kDelete },
   { "Home", kHome },
   { "End", kEnd },
   { "PageUp", kPageUp },
   { "PageDown", kPageDown },
   { "Tab", kTab },
   { "Backspace", kBackspace },
   { "Enter", kEnter },
   { "Ctrl", kCtrl },
   { "LeftShift", kLShift },
   { "RightShift", kRShift },
   { "Alt", kAlt },
   { "Up", kUp },
   { "Down", kDown },
   { "Left", kLeft },
   { "Right", kRight },
   { "Escape", kEscape },
   { "Esc", kEscape },
   { "Space", kSpace },
   { "Pause", kPause },
   { "MouseLeft", kMouseLeft },
   { "MouseMiddle", kMouseMiddle },
   { "MouseRight", kMouseRight },
   { "MouseWheelUp", kMouseWheelUp },
   { "MouseWheelDown", kMouseWheelDown },
   { "MouseMove", kMouseMove },
};

static const size_t g_nNamedKeys = _countof(g_keyNames);

///////////////////////////////////////////////////////////////////////////////

const char * Key2Name(long key)
{
   for (int i = 0; i < g_nNamedKeys; i++)
   {
      if (g_keyNames[i].keyCode == key)
      {
         return g_keyNames[i].pszKeyName;
      }
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////

long Name2Key(const char * pszKeyName)
{
   for (int i = 0; i < g_nNamedKeys; i++)
   {
      if (stricmp(g_keyNames[i].pszKeyName, pszKeyName) == 0)
      {
         return g_keyNames[i].keyCode;
      }
   }
   if (*pszKeyName > 0)
   {
      return *pszKeyName;
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

int BindKey(int argc, const tScriptVar * argv, 
            int nMaxResults, tScriptVar * pResults)
{
   const char * pszKeyName = NULL;
   const char * pszDownCmd = NULL;
   const char * pszUpCmd = NULL;

   if (argc == 2)
   {
      if (ScriptArgIsString(0))
      {
         pszKeyName = argv[0];
      }
      if (ScriptArgIsString(1))
      {
         pszDownCmd = argv[1];
      }
   }
   else if (argc == 3)
   {
      if (ScriptArgIsString(0))
      {
         pszKeyName = argv[0];
      }
      if (ScriptArgIsString(1))
      {
         pszDownCmd = argv[1];
      }
      if (ScriptArgIsString(2))
      {
         pszUpCmd = argv[2];
      }
   }

   if (pszKeyName != NULL && (pszDownCmd != NULL || pszUpCmd != NULL))
   {
      UseGlobal(Input);
      if (!!pInput)
      {
         pInput->KeyBind(Name2Key(pszKeyName), pszDownCmd, pszUpCmd);
      }
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

int UnbindKey(int argc, const tScriptVar * argv, 
              int nMaxResults, tScriptVar * pResults)
{
   if (argc == 1 && ScriptArgIsString(0))
   {
      UseGlobal(Input);
      if (!!pInput)
      {
         pInput->KeyUnbind(Name2Key(argv[0]));
      }
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

int Quit(int argc, const tScriptVar * argv, 
         int nMaxResults, tScriptVar * pResults)
{
   SysQuit();
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

int LogEnableChannel(int argc, const tScriptVar * argv, 
                     int nMaxResults, tScriptVar * pResults)
{
   if (argc == 1 && ScriptArgIsString(0))
   {
      techlog.EnableChannel(argv[0], true);
   }
   else if (argc == 2 && ScriptArgIsString(0) && ScriptArgIsNumber(1))
   {
      techlog.EnableChannel(argv[0], ((int)argv[1] != 0) ? true : false);
   }
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

int EntitySpawnTest(int argc, const tScriptVar * argv, 
                    int nMaxResults, tScriptVar * pResults)
{
   if (argc == 3
      && ScriptArgIsString(0)
      && ScriptArgIsNumber(1)
      && ScriptArgIsNumber(2))
   {
      float x = argv[1];
      float z = argv[2];

      if (x < 0 || x > 1 || z < 0 || z > 1)
      {
         ErrorMsg2("EntitySpawnTest arguments %f, %f, out of range\n", x, z);
      }
      else
      {
         UseGlobal(EntityManager);
         pEntityManager->SpawnEntity(argv[0], argv[1], argv[2]);
      }
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ListResources(int argc, const tScriptVar * argv, 
                  int nMaxResults, tScriptVar * pResults)
{
   if (argc < 1 || !argv[0].IsString())
   {
      ErrorMsg("Invalid parameters to ListResources\n");
      return 0;
   }

   bool bDontLoad = false;
   if (argc > 1 && ScriptArgIsNumber(1))
   {
      bDontLoad = (argv[1].ToInt() != 0);
   }

   cAutoIPtr<IDictionary> pDict(DictionaryCreate());
   if (!pDict)
   {
      return 0;
   }

   std::vector<cStr> resources;
   UseGlobal(ResourceManager);
   if (pResourceManager->ListResources(argv[0], &resources) == S_OK)
   {
      std::vector<cStr>::iterator iter = resources.begin();
      for (int index = 0; iter != resources.end(); iter++, index++)
      {
         void * pTemp = NULL;
         if (bDontLoad || (pResourceManager->Load(iter->c_str(), argv[0], NULL, &pTemp) == S_OK))
         {
            cStr key;
            Sprintf(&key, _T("%d"), index + 1);
            pDict->Set(key.c_str(), iter->c_str());
         }
      }
   }

   pResults[0] = pDict;
   return 1;
}


///////////////////////////////////////////////////////////////////////////////

int GetMapProperties(int argc, const tScriptVar * argv, 
                     int nMaxResults, tScriptVar * pResults)
{
   if (argc < 1 || !argv[0].IsString())
   {
      ErrorMsg("Invalid parameters to GetMapProperties\n");
      return 0;
   }

   void * pUnused = NULL;
   cMapProperties mapProperties;
   UseGlobal(ResourceManager);
   pResourceManager->Load(argv[0], kRT_Map, &mapProperties, &pUnused);
   // Ignore the return value because when loading just the properties instead
   // of the full map itself, the loader returns NULL to prevent the resource
   // manager from caching the wrong thing.

   if (nMaxResults >= 4 && _tcslen(mapProperties.GetTitle()) > 0)
   {
      pResults[0] = mapProperties.GetTitle();
      pResults[1] = mapProperties.GetAuthor();
      pResults[2] = mapProperties.GetDescription();
      pResults[3] = mapProperties.GetNumPlayers();
      return 4;
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

sScriptReg cmds[] =
{
   { "bind", BindKey },
   { "unbind", UnbindKey },
   { "quit", Quit },
   { "LogChannel", LogEnableChannel },
   { "EntitySpawnTest", EntitySpawnTest },
   { "ListResources", ListResources },
   { "GetMapProperties", GetMapProperties },
};

ENGINE_API tResult EngineRegisterScriptFunctions()
{
   return ScriptAddFunctions(cmds, _countof(cmds));
}

///////////////////////////////////////////////////////////////////////////////
