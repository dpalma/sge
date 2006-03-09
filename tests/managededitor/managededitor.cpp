///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorForm.h"

#include "guiapi.h"

#include "cameraapi.h"
#include "engineapi.h"
#include "entityapi.h"
#include "inputapi.h"
#include "renderapi.h"
#include "saveloadapi.h"
#include "scriptapi.h"
#include "simapi.h"
#include "sys.h"
#include "terrainapi.h"

#include "configapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"
#include "imageapi.h"
#include "resourceapi.h"
#include "techstring.h"
#include "threadcallapi.h"

#include <ctime>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   CameraCreate();
   EntityManagerCreate();
   GUIContextCreate();
   GUIFactoryCreate();
   GUIFontFactoryCreate();
   InputCreate();
   RendererCreate();
   ResourceManagerCreate();
   SaveLoadManagerCreate();
   SimCreate();
   ScriptInterpreterCreate();
   TerrainModelCreate();
   TerrainRendererCreateForEditor();
   ThreadCallerCreate();
}


///////////////////////////////////////////////////////////////////////////////

static tResult InitGlobalConfig(int argc, tChar * argv[])
{
   Assert(argc > 0);

   cFileSpec cfgFile(argv[0]);
   cfgFile.SetFileExt(_T("cfg"));

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(cfgFile);
   if (!pStore)
   {
      return E_OUTOFMEMORY;
   }
   pStore->Load(g_pConfig);

   ParseCommandLine(argc, argv, g_pConfig);

   return S_OK;
}

static bool ManagedEditorInit(int argc, tChar * argv[])
{
   if (InitGlobalConfig(argc, argv) != S_OK)
   {
      return false;
   }

   srand(GetTickCount());
   SeedRand(GetTickCount());

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      DebugMsg("One or more application-level services failed to start!\n");
      return false;
   }

   TextFormatRegister(NULL);
   EngineRegisterResourceFormats();
   TerrainRegisterResourceFormats();
   ImageRegisterResourceFormats();

   cStr temp;
   if (ConfigGet(_T("data"), &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddDirectoryTreeFlattened(temp.c_str());
   }

   UseGlobal(GUIContext);
   pGUIContext->PushPage("start.xml");

   UseGlobal(Sim);
   pSim->Go();

#if 0
   if (FAILED(SysRunUnitTests()))
   {
      return false;
   }
#else
   SysRunUnitTests();
#endif

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static void ManagedEditorTerm()
{
   UseGlobal(Sim);
   pSim->Stop();

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

int STDCALL _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPTSTR lpCmdLine, int nShowCmd)
{
	// Enabling Windows XP visual effects before any controls are created
	//Application::EnableVisualStyles();
	//Application::SetCompatibleTextRenderingDefault(false); 

   if (!ManagedEditorInit(__argc, __targv))
   {
      ManagedEditorTerm();
      return -1;
   }

   System::Windows::Forms::Application::Run(gcnew Editor::cEditorForm());

   ManagedEditorTerm();

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
