/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorapi.h"

#include "sceneapi.h"
#include "inputapi.h"
#include "scriptapi.h"

#include "textureapi.h"

#include "resmgr.h"
#include "threadcallapi.h"
#include "configapi.h"
#include "globalobj.h"
#include "filespec.h"
#include "filepath.h"

#include "resource.h"       // main symbols

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

CAppModule _Module;

/////////////////////////////////////////////////////////////////////////////

BEGIN_OBJECT_MAP(g_objectMap)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   InputCreate();
//   SimCreate();
   ResourceManagerCreate();
   SceneCreate();
   ScriptInterpreterCreate();
   TextureManagerCreate();
//   GUIContextCreate();
//   GUIFactoryCreate();
//   GUIRenderingToolsCreate();
   EditorTileManagerCreate();
   ThreadCallerCreate();
   EditorAppCreate();
}

/////////////////////////////////////////////////////////////////////////////

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                     LPTSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
   CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

   // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
   DefWindowProc(NULL, 0, 0, 0L);

   AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);

   if (FAILED(_Module.Init(g_objectMap, hInstance)))
   {
      ErrorMsg("ATL module failed to start!\n");
      return -1;
   }

   CMessageLoop messageLoop;
   _Module.AddMessageLoop(&messageLoop);

   cFileSpec file(__argv[0]);
   file.SetPath(cFilePath());
   file.SetFileExt("cfg");

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(file);
   if (pStore->Load(g_pConfig) != S_OK)
   {
      WarnMsg1("Error loading settings from %s\n", file.GetName());
   }

   ParseCommandLine(__argc, __argv, g_pConfig);

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      ErrorMsg("One or more application-level services failed to start!\n");
      return -1;
   }

   // Prepare the main thread for use by the thread caller
   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadInit();

   int result = messageLoop.Run();

	StopGlobalObjects();

   _Module.RemoveMessageLoop();
   _Module.Term();
   CoUninitialize();

   return result;
}

/////////////////////////////////////////////////////////////////////////////
