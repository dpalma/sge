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

static tResult InitGlobalConfig(array<System::String ^> ^ args)
{
#if 0 // TODO
   cFileSpec cfgFile(argv[0]);
   cfgFile.SetFileExt(_T("cfg"));

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(cfgFile);
   if (!pStore)
   {
      return E_OUTOFMEMORY;
   }
   pStore->Load(g_pConfig);
#endif

   using namespace System::Runtime::InteropServices;

   for (int i = 0; i < args->Length; i++)
   {
      System::String ^ arg = args[i];
#ifndef _UNICODE
      System::IntPtr pArg = Marshal::StringToHGlobalAnsi(arg);
      char * argv[] = { static_cast<char *>(pArg.ToPointer()) };
#else
      pin_ptr<const wchar_t> psz = PtrToStringChars(arg);
      wchar_t * argv[] = { psz };
#endif
      ParseCommandLine(1, argv, g_pConfig);
#ifndef _UNICODE
      Marshal::FreeHGlobal(pArg);
#endif
   }

   return S_OK;
}

static bool ManagedEditorInit(array<System::String ^> ^ args)
{
   if (InitGlobalConfig(args) != S_OK)
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

int main(array<System::String ^> ^ args)
{
   if (!ManagedEditorInit(args))
   {
      ManagedEditorTerm();
      return -1;
   }

   System::Windows::Forms::Application::Run(gcnew ManagedEditor::EditorForm());

   ManagedEditorTerm();

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
