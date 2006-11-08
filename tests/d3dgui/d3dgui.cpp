///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "gui/guiapi.h"
#include "gui/guielementapi.h"

#include "engine/engineapi.h"
#include "platform/inputapi.h"
#include "platform/sys.h"
#include "render/renderapi.h"
#include "script/scriptapi.h"

#include "tech/resourceapi.h"
#include "tech/configapi.h"
#include "tech/filespec.h"
#include "tech/filepath.h"
#include "tech/imageapi.h"
#include "tech/techstring.h"
#include "tech/globalobj.h"
#include "tech/threadcallapi.h"
#include "tech/schedulerapi.h"

#include <ctime>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "tech/dbgalloc.h" // must be last header

#pragma comment(lib, "d3d9")
#ifdef D3D_DEBUG_INFO
#pragma comment(lib, "d3dx9d")
#else
#pragma comment(lib, "d3dx9")
#endif

// The following definitions are required for WinMain
F_DECLARE_HANDLE(HINSTANCE);
typedef char * LPSTR;

static tResult d3dguiframe();

///////////////////////////////////////////////////////////////////////////////

#define kDefaultWidth   800
#define kDefaultHeight  600
#define kDefaultBpp     16
#define kDefaultFov     70

static const tChar g_szFrameStatsOverlay[] = _T("<page><label renderer=\"basic\" id=\"frameStats\" style=\"width:50%;height:25%;foreground-color:white\" /></page>");

///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<IGUILabelElement> g_pFrameStats;

HWND g_hWnd = NULL;


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   InputCreate();
   SchedulerCreate();
   ResourceManagerCreate();
   ScriptInterpreterCreate();
   GUIContextCreate();
   GUIFactoryCreate();
   ThreadCallerCreate();
   RendererCreate();
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

static bool d3dguiinit(int argc, tChar * argv[])
{
   if (InitGlobalConfig(argc, argv) != S_OK)
   {
      return false;
   }

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      DebugMsg("One or more application-level services failed to start!\n");
      return false;
   }

   ImageRegisterResourceFormats();
   TextFormatRegister(NULL);
   EngineRegisterResourceFormats();
   EngineRegisterScriptFunctions();

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

   g_hWnd = reinterpret_cast<HWND>(SysCreateWindow(_T("D3D GUI Test"), width, height, kDirect3D9));
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
   if (pGUIContext->AddOverlayPage(g_szFrameStatsOverlay) == S_OK)
   {
      cAutoIPtr<IGUIElement> pElement;
      if (pGUIContext->GetOverlayElement(_T("frameStats"), &pElement) == S_OK)
      {
         pElement->QueryInterface(IID_IGUILabelElement, (void**)&g_pFrameStats);
      }
   }

   pGUIContext->PushPage("start.xml");

   SysSetFrameCallback(d3dguiframe);
   SysAppActivate(true);

   UseGlobal(Scheduler);
   pScheduler->Start();

   if (FAILED(SysRunUnitTests()))
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static void d3dguiterm()
{
   UseGlobal(Scheduler);
   pScheduler->Stop();

   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   // This will make sure the GL context is destroyed
   SysQuit();

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

static tResult d3dguiframe()
{
   if (!!g_pFrameStats)
   {
      tChar szStats[100];
      SysReportFrameStats(szStats, _countof(szStats));

      g_pFrameStats->SetText(szStats);
   }

   UseGlobal(Renderer);
   if (pRenderer->BeginScene() == S_OK)
   {
      int width, height;
      if (SysGetWindowSize(&width, &height) == S_OK)
      {
         cAutoIPtr<IRender2D> pRender2D;
         if (pRenderer->Begin2D(width, height, &pRender2D) == S_OK)
         {
            UseGlobal(GUIContext);
            pGUIContext->RenderGUI(pRender2D);

            pRenderer->End2D();
         }
      }

      pRenderer->EndScene();
   }

   SysSwapBuffers();

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) && !defined(__CYGWIN__)
int STDCALL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nShowCmd)
{
   if (!d3dguiinit(__argc, __targv))
   {
      d3dguiterm();
      return -1;
   }

   int result = SysEventLoop(d3dguiframe, kSELF_RunScheduler);

   d3dguiterm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(__CYGWIN__)
int main(int argc, char * argv[])
{
   if (!d3dguiinit(argc, argv))
   {
      d3dguiterm();
      return EXIT_FAILURE;
   }

   int result = SysEventLoop(NULL, kSELF_RunScheduler);

   d3dguiterm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////
