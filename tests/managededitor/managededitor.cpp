///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorAppForm.h"

#include "guiapi.h"

#include "cameraapi.h"
#include "engineapi.h"
#include "entityapi.h"
#include "inputapi.h"
#include "renderapi.h"
#include "saveloadapi.h"
#include "schedulerapi.h"
#include "scriptapi.h"
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
   CameraControlCreate();
   EntityManagerCreate();
   GUIContextCreate();
   GUIFactoryCreate();
   GUIFontCacheCreate();
   InputCreate();
   RendererCreate();
   ResourceManagerCreate();
   SaveLoadManagerCreate();
   SchedulerCreate();
   ScriptInterpreterCreate();
   TerrainModelCreate();
   TerrainRendererCreateForEditor();
   ThreadCallerCreate();
}


///////////////////////////////////////////////////////////////////////////////

void StringConvert(System::String ^ string, cStr * pStr)
{
#ifndef _UNICODE
   using namespace System::Runtime::InteropServices;
   System::IntPtr pString = Marshal::StringToHGlobalAnsi(string);
   pStr->assign(static_cast<char *>(pString.ToPointer()));
   Marshal::FreeHGlobal(pString);
#else
   pin_ptr<const wchar_t> psz = PtrToStringChars(string);
   pStr->assign(psz);
#endif
}

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

   for (int i = 0; i < args->Length; i++)
   {
      cStr arg;
      StringConvert(args[i], &arg);
      tChar * argv[] = { const_cast<tChar*>(arg.c_str()) };
      ParseCommandLine(1, argv, g_pConfig);
   }

   return S_OK;
}

static bool ManagedEditorInit(array<System::String ^> ^ args)
{
   if (InitGlobalConfig(args) != S_OK)
   {
      return false;
   }

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      DebugMsg("One or more application-level services failed to start!\n");
      return false;
   }

   TextFormatRegister(_T("css,lua,xml"));
   EngineRegisterResourceFormats();
   EngineRegisterScriptFunctions();
   TerrainRegisterResourceFormats();
   ImageRegisterResourceFormats();

   cStr temp;
   if (ConfigGet(_T("data"), &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddDirectoryTreeFlattened(temp.c_str());
   }

   cAutoIPtr<IGUIRenderDeviceContext> pGuiRenderDevice;
   if (GUIRenderDeviceCreateGL(&pGuiRenderDevice) == S_OK)
   {
      UseGlobal(GUIContext);
      pGUIContext->SetRenderDeviceContext(pGuiRenderDevice);
   }

   UseGlobal(Scheduler);
   pScheduler->Start();

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
   UseGlobal(Scheduler);
   pScheduler->Stop();

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

   System::Windows::Forms::Application::Run(gcnew ManagedEditor::EditorAppForm());

   ManagedEditorTerm();

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
