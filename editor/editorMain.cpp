/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorapi.h"
#include "terrainapi.h"
#include "editorMessageLoop.h"

#if _ATL_VER < 0x0700
// header files required to build ATL 3.0 object map
#include "editorDoc.h"
#include "editorView.h"
#endif

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

#if _ATL_VER < 0x0700
BEGIN_OBJECT_MAP(g_objectMap)
   OBJECT_ENTRY(CLSID_EditorDoc, cEditorDoc)
   OBJECT_ENTRY(CLSID_EditorView, cEditorView)
END_OBJECT_MAP()
static _ATL_OBJMAP_ENTRY * GetObjectMap()
{
   return g_objectMap;
}
#else
static _ATL_OBJMAP_ENTRY * GetObjectMap()
{
   return NULL;
}
#endif

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
   TerrainRendererCreate();
}

/////////////////////////////////////////////////////////////////////////////

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                     LPTSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
   CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

   // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
   DefWindowProc(NULL, 0, 0, 0L);

   AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);

   if (FAILED(_Module.Init(GetObjectMap(), hInstance)))
   {
      ErrorMsg("ATL module failed to start!\n");
      return -1;
   }

   if (FAILED(_Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED)))
   {
      ErrorMsg("ATL module failed to register class objects!\n");
      return -1;
   }

   if (FAILED(CoResumeClassObjects()))
   {
      ErrorMsg("Failed to resume class objects!\n");
      return -1;
   }

   cEditorMessageLoop messageLoop;
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
   _Module.RevokeClassObjects();
   _Module.Term();
   CoUninitialize();

   return result;
}

/////////////////////////////////////////////////////////////////////////////
