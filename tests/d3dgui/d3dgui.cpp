///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guiapi.h"
#include "sim.h"
#include "inputapi.h"
#include "engineapi.h"
#include "scriptapi.h"
#include "scriptvar.h"
#include "sys.h"

#include "techmath.h"
#include "resourceapi.h"
#include "configapi.h"
#include "filespec.h"
#include "filepath.h"
#include "techstring.h"
#include "globalobj.h"
#include "readwriteapi.h"
#include "threadcallapi.h"
#include "techtime.h"

#include <ctime>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestFailure.h>
#include <cppunit/SourceLine.h>
#include <cppunit/Exception.h>
#include <cppunit/ui/text/TestRunner.h>
#endif

#define VC_EXTRALEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' used in base member initializer list

#pragma comment(lib, "d3d9")
#pragma comment(lib, "d3dx9")

// The following definitions are required for WinMain
F_DECLARE_HANDLE(HINSTANCE);
typedef char * LPSTR;

extern tResult GUIRenderDeviceCreateD3D(IDirect3DDevice9 * pD3dDevice, IGUIRenderDeviceContext * * ppRenderDevice);
extern void GUIFontFactoryCreateD3D();

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
cAutoIPtr<IDirect3D9> g_pD3d;
cAutoIPtr<IDirect3DDevice9> g_pD3dDevice;

///////////////////////////////////////////////////////////////////////////////

static tResult InitD3D(HWND hWnd, IDirect3D9 * * ppD3d, IDirect3DDevice9 * * ppDevice)
{
   if (!IsWindow(hWnd))
   {
      return E_INVALIDARG;
   }

   if (ppD3d == NULL || ppDevice == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IDirect3D9> pD3d(Direct3DCreate9(DIRECT3D_VERSION));
   if (!pD3d)
   {
      return E_FAIL;
   }

   D3DDISPLAYMODE displayMode;
   if (FAILED(pD3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode)))
   {
      return E_FAIL;
   }

   D3DPRESENT_PARAMETERS presentParams;
   memset(&presentParams, 0, sizeof(presentParams));
   presentParams.BackBufferCount = 1;
   presentParams.BackBufferFormat = displayMode.Format;
   presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
   presentParams.Windowed = TRUE;
   presentParams.EnableAutoDepthStencil = TRUE;
   presentParams.AutoDepthStencilFormat = D3DFMT_D16;
   presentParams.hDeviceWindow = hWnd;
   presentParams.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL | D3DPRESENTFLAG_DEVICECLIP;

   cAutoIPtr<IDirect3DDevice9> pD3dDevice;
   HRESULT hr = pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
      D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &pD3dDevice);
   if (FAILED(hr))
   {
      hr = pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
         D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &pD3dDevice);
      {
         ErrorMsg1("D3D error %x\n", hr);
         return hr;
      }
   }

   *ppD3d = CTAddRef(pD3d);
   *ppDevice = CTAddRef(pD3dDevice);

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT
static bool RunUnitTests()
{
   CppUnit::TextUi::TestRunner runner;
   runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
   runner.run();
   if (runner.result().testFailuresTotal() > 0)
   {
      techlog.Print(kError, "%d UNIT TESTS FAILED!\n", runner.result().testFailuresTotal());
      CppUnit::TestResultCollector::TestFailures::const_iterator iter;
      for (iter = runner.result().failures().begin(); iter != runner.result().failures().end(); iter++)
      {
         techlog.Print(kError, "%s(%d) : %s : %s\n",
            (*iter)->sourceLine().fileName().c_str(),
            (*iter)->sourceLine().isValid() ? (*iter)->sourceLine().lineNumber() : -1,
            (*iter)->failedTestName().c_str(),
            (*iter)->thrownException()->what());
      }
      return false;
   }
   else
   {
      techlog.Print(kInfo, "%d unit tests succeeded\n", runner.result().tests().size());
      return true;
   }
}
#endif


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   InputCreate();
   SimCreate();
   ResourceManagerCreate();
   ScriptInterpreterCreate();
   GUIContextCreate();
   GUIFactoryCreate();
   GUIFontFactoryCreateD3D();
   ThreadCallerCreate();
}

///////////////////////////////////////////////////////////////////////////////

static double fpsLast = 0;
static double fpsWorst = 99999;//DBL_MAX;
static double fpsBest = 0;
static double fpsAverage = 0;

static double FPS()
{
   static double lastTime = 0;
   static double frameCount = 0;

   double time = TimeGetSecs();
   double elapsed = time - lastTime;
   frameCount++;

   double fps = 0;
   if (elapsed >= 0.5) // update about 2x per second
   {
      if (lastTime != 0.0)
      {
         double fps = frameCount / elapsed;
         if (fpsAverage == 0)
         {
            fpsAverage = fps;
         }
         else
         {
            fpsAverage = (fps + fpsLast) * 0.5;
         }
         if (fps > fpsBest)
         {
            fpsBest = fps;
         }
         if (fps < fpsWorst)
         {
            fpsWorst = fps;
         }
         fpsLast = fps;
      }
      lastTime = time;
      frameCount = 0;
   }
   return fps;
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

   srand(time(NULL));
   SeedRand(time(NULL));

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      DebugMsg("One or more application-level services failed to start!\n");
      return false;
   }

   TargaFormatRegister();
   BmpFormatRegister();
   TextFormatRegister("txt");
   TextFormatRegister("lua");
   TextFormatRegister("xml");
   EngineRegisterResourceFormats();

   cStr temp;
   if (ConfigGet(_T("data"), &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddDirectoryTreeFlattened(temp.c_str());
   }

   int width = kDefaultWidth;
   int height = kDefaultHeight;
   int bpp = kDefaultBpp;
   ConfigGet(_T("screen_width"), &width);
   ConfigGet(_T("screen_height"), &height);
   ConfigGet(_T("screen_bpp"), &bpp);

   g_hWnd = reinterpret_cast<HWND>(SysCreateWindow(_T("D3D GUI Test"), width, height));
   if (g_hWnd == NULL)
   {
      return false;
   }

   if (InitD3D(g_hWnd, &g_pD3d, &g_pD3dDevice) != S_OK)
   {
      return false;
   }

   UseGlobal(GUIFontFactory);
   cAutoIPtr<IGUIFontFactoryD3D> pGUIFontFactoryD3D;
   if (pGUIFontFactory->QueryInterface(IID_IGUIFontFactoryD3D, (void**)&pGUIFontFactoryD3D) == S_OK)
   {
      pGUIFontFactoryD3D->SetD3DDevice(g_pD3dDevice);
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

   pGUIContext->LoadElements("start.xml", true);

   cAutoIPtr<IGUIRenderDeviceContext> pGuiRenderDevice;
   if (GUIRenderDeviceCreateD3D(g_pD3dDevice, &pGuiRenderDevice) != S_OK)
   {
      return false;
   }

   pGUIContext->SetRenderDeviceContext(pGuiRenderDevice);

   SysAppActivate(true);

   UseGlobal(Sim);
   pSim->Go();

#ifdef HAVE_CPPUNIT
   if (!RunUnitTests())
   {
      return false;
   }
#endif

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static void d3dguiterm()
{
   UseGlobal(Sim);
   pSim->Stop();

   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   SafeRelease(g_pFont);

   SafeRelease(g_pD3d);
   SafeRelease(g_pD3dDevice);

   // This will make sure the GL context is destroyed
   SysQuit();

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

static bool d3dguiframe()
{
   UseGlobal(Sim);
   pSim->NextFrame();

   if (!g_pD3dDevice)
   {
      // D3D device not initialized yet
      return true;
   }

   g_pD3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1, 0);

   if (g_pD3dDevice->BeginScene() == D3D_OK)
   {
      UseGlobal(GUIContext);
      cAutoIPtr<IGUIRenderDeviceContext> pRenderDeviceContext;
      if (pGUIContext->GetRenderDeviceContext(&pRenderDeviceContext) == S_OK)
      {
         pRenderDeviceContext->Begin2D();

         pGUIContext->RenderGUI();

         if (!!g_pFont)
         {
            FPS();

            char szStats[100];
            snprintf(szStats, _countof(szStats),
               "%.2f fps\n"
               "%.2f worst\n"
               "%.2f best\n"
               "%.2f average",
               fpsLast, 
               fpsWorst,
               fpsBest, 
               fpsAverage);

            tRect rect(kDefStatsX, kDefStatsY, 0, 0);
            g_pFont->RenderText(szStats, strlen(szStats), &rect, kRT_NoClip | kRT_DropShadow, kDefStatsColor);
         }

         pRenderDeviceContext->End2D();
      }

      g_pD3dDevice->EndScene();
      g_pD3dDevice->Present(NULL, NULL, NULL, NULL);
   }

//   SysSwapBuffers();

   return true;
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

   int result = SysEventLoop(d3dguiframe, NULL);

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

   int result = SysEventLoop(d3dguiframe, NULL);

   d3dguiterm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////
