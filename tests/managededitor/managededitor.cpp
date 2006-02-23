///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

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

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/TestFactoryRegistry.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

#define kDefaultWidth   800
#define kDefaultHeight  600
#define kDefaultBpp     16
#define kDefaultFov     70

static const int kDefStatsX = 25;
static const int kDefStatsY = 25;
static const cColor kDefStatsColor(1,1,1,1);

///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<IGUIFont> g_pFont;

HWND g_hWnd = NULL;


///////////////////////////////////////////////////////////////////////////////

static tResult ManagedEditorFrame()
{
   UseGlobal(Sim);
   pSim->NextFrame();

   UseGlobal(Renderer);
   if (pRenderer->BeginScene() != S_OK)
   {
      return E_FAIL;
   }

   UseGlobal(GUIContext);
   cAutoIPtr<IGUIRenderDeviceContext> pRenderDeviceContext;
   if (pGUIContext->GetRenderDeviceContext(&pRenderDeviceContext) == S_OK)
   {
      pRenderDeviceContext->Begin2D();

      pGUIContext->RenderGUI();

      if (!!g_pFont)
      {
         tChar szStats[100];
         SysReportFrameStats(szStats, _countof(szStats));

         tRect rect(kDefStatsX, kDefStatsY, 0, 0);
         g_pFont->RenderText(szStats, _tcslen(szStats), &rect,
                             kRT_NoClip | kRT_DropShadow, kDefStatsColor);
      }

      pRenderDeviceContext->End2D();
   }

   pRenderer->EndScene();
   SysSwapBuffers();

   return S_OK;
}


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
   ThreadCallerCreate();
   TerrainModelCreate();
   TerrainRendererCreateForEditor();
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

   srand(time(NULL));
   SeedRand(time(NULL));

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

   cStr script("autoexec.lua");
   ConfigGet(_T("autoexec_script"), &script);
   if (!script.empty())
   {
      char * pszCode = NULL;
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(script.c_str(), kRT_Text, NULL, (void**)&pszCode) == S_OK)
      {
         UseGlobal(ScriptInterpreter);
         pScriptInterpreter->ExecString(pszCode);
      }
   }

   int width = kDefaultWidth;
   int height = kDefaultHeight;
   int bpp = kDefaultBpp;
   ConfigGet(_T("screen_width"), &width);
   ConfigGet(_T("screen_height"), &height);
   ConfigGet(_T("screen_bpp"), &bpp);

   g_hWnd = reinterpret_cast<HWND>(SysCreateWindow(_T("Managed Editor"), width, height));
   if (g_hWnd == NULL)
   {
      return false;
   }

   UseGlobal(ThreadCaller);
   if (FAILED(pThreadCaller->ThreadInit()))
   {
      return false;
   }

   UseGlobal(GUIContext);
   if (FAILED(pGUIContext->GetDefaultFont(&g_pFont)))
   {
      WarnMsg("Failed to get a default font interface pointer for showing frame stats\n");
      return false;
   }

   pGUIContext->PushPage("start.xml");

   SysSetFrameCallback(ManagedEditorFrame);
   SysAppActivate(true);

   UseGlobal(Sim);
   pSim->Go();

#ifdef HAVE_CPPUNIT
   if (FAILED(SysRunUnitTests()))
   {
      return false;
   }
#endif

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static void ManagedEditorTerm()
{
   UseGlobal(Sim);
   pSim->Stop();

   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   SafeRelease(g_pFont);

   // This will make sure the GL context is destroyed
   SysQuit();

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

static void OnApplicationIdle(System::Object ^ sender, System::EventArgs ^ e)
{
   MSG msg;
   while (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
   {
      ManagedEditorFrame();
   }
}

///////////////////////////////////////////////////////////////////////////////

ref class cManagedEditorForm : public System::Windows::Forms::Form
{
public:
   cManagedEditorForm();
};

cManagedEditorForm::cManagedEditorForm()
{
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

   System::Windows::Forms::Application::Idle += gcnew System::EventHandler(OnApplicationIdle);

   System::Windows::Forms::Application::Run(gcnew cManagedEditorForm());

   //int result = SysEventLoop(ManagedEditorFrame);

   ManagedEditorTerm();

   //return result;

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
