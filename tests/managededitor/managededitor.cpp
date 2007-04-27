///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorAppForm.h"

#include "gui/guiapi.h"

#include "engine/cameraapi.h"
#include "engine/engineapi.h"
#include "engine/entityapi.h"
#include "engine/saveloadapi.h"
#include "engine/terrainapi.h"
#include "platform/inputapi.h"
#include "platform/sys.h"
#include "render/renderapi.h"
#include "script/scriptapi.h"

#include "tech/configapi.h"
#include "tech/filespec.h"
#include "tech/filepath.h"
#include "tech/globalobj.h"
#include "tech/imageapi.h"
#include "tech/resourceapi.h"
#include "tech/schedulerapi.h"
#include "tech/techstring.h"
#include "tech/threadcallapi.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   CameraControlCreate();
   EntityComponentRegistryCreate();
   EntityManagerCreate();
   GUIContextCreate();
   GUIFactoryCreate();
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

int main(array<System::String ^> ^ args)
{
   if (!ManagedEditorInit(args))
   {
      StopGlobalObjects();
      return -1;
   }

   System::Windows::Forms::Application::Run(gcnew ManagedEditor::EditorAppForm());

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
