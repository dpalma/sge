///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "script.h"
#include "cameracontroller.h"

#include "cameraapi.h"
#include "guiapi.h"
#include "inputapi.h"
#include "engineapi.h"
#include "entityapi.h"
#include "renderapi.h"
#include "saveloadapi.h"
#include "simapi.h"
#include "soundapi.h"
#include "sys.h"
#include "terrainapi.h"

#include "resourceapi.h"
#include "configapi.h"
#include "filespec.h"
#include "filepath.h"
#include "techstring.h"
#include "globalobj.h"
#include "multivar.h"
#include "threadcallapi.h"
#include "imageapi.h"

#include <ctime>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/TestFactoryRegistry.h>
#endif

#include "dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' used in base member initializer list

// The following definitions are required for WinMain
F_DECLARE_HANDLE(HINSTANCE);
typedef char * LPSTR;

///////////////////////////////////////////////////////////////////////////////

#define kAutoExecScript _T("autoexec.lua")
#define kDefaultWidth   800
#define kDefaultHeight  600
#define kDefaultBpp     16
#define kDefaultFov     70

const float kZNear = 1;
const float kZFar = 2000;

static const float kGroundScaleY = 0.25f;

static const int kDefStatsX = 25;
static const int kDefStatsY = 25;
static const cColor kDefStatsColor(1,1,1,1);

static const tChar kRT_MapFile[] = _T("SGE.Map");

///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<cGameCameraController> g_pGameCameraController;

cAutoIPtr<IGUIFont> g_pFont;

float g_fov;


///////////////////////////////////////////////////////////////////////////////

inline bool IsNumber(const tScriptVar & scriptVar)
{
   return scriptVar.IsInt() || scriptVar.IsFloat() || scriptVar.IsDouble();
}

SCRIPT_DEFINE_FUNCTION(ViewSetPos)
{
   if (argc == 2 && IsNumber(argv[0]) && IsNumber(argv[1]))
   {
      float x = argv[0];
      float z = argv[1];

      if (x >= 0 && x <= 1 && z >= 0 && z <= 1)
      {
         cTerrainSettings terrainSettings;
         UseGlobal(TerrainModel);
         pTerrainModel->GetTerrainSettings(&terrainSettings);
         x *= terrainSettings.GetTileCountX() * terrainSettings.GetTileSize();
         z *= terrainSettings.GetTileCountZ() * terrainSettings.GetTileSize();
         g_pGameCameraController->LookAtPoint(x, z);
      }
      else
      {
         DebugMsg2("ViewSetPos arguments %f, %f, out of range\n", x, z);
      }
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

void ResizeHack(int width, int height)
{
   UseGlobal(Camera);
   if (!!pCamera)
   {
      pCamera->SetPerspective(g_fov, static_cast<float>(width) / height, kZNear, kZFar);
   }
}


///////////////////////////////////////////////////////////////////////////////

void * HackScenarioLoad(IReader * pReader)
{
   UseGlobal(SaveLoadManager);
   if (pSaveLoadManager->Load(pReader) == S_OK)
   {
      return reinterpret_cast<void*>(NULL + 1);
   }
   return NULL;
}

void HackScenarioUnload(void * pData)
{
}

static void HackScenarioRegisterResourceFormat()
{
   UseGlobal(ResourceManager);
   pResourceManager->RegisterFormat(kRT_MapFile, "sgm", HackScenarioLoad, NULL, HackScenarioUnload);
}


///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(SetTerrain)
{
   if (argc < 1 || !argv[0].IsString())
   {
      ErrorMsg("Invalid parameters to SetTerrain\n");
      return 0;
   }

   UseGlobal(ResourceManager);

   cFileSpec terrainFile(static_cast<const tChar *>(argv[0]));
   const tChar * pszFileExt = terrainFile.GetFileExt();
   if (pszFileExt != NULL && _tcsicmp(pszFileExt, _T("sgm")) == 0)
   {
      void * pData = NULL;
      pResourceManager->Load(argv[0], kRT_MapFile, NULL, &pData);
   }
   else
   {
      UseGlobal(TerrainModel);

      cTerrainSettings terrainSettings;
      terrainSettings.SetTileSize(32);
      terrainSettings.SetTileCountX(64);
      terrainSettings.SetTileCountZ(64);
      terrainSettings.SetTileSet("defaulttiles.xml"); // HACK TODO

      if (argc == 1 && argv[0].IsString())
      {
         terrainSettings.SetHeightData(kTHD_HeightMap);
         terrainSettings.SetHeightMap(argv[0]);
         terrainSettings.SetHeightMapScale(kGroundScaleY * 255);
         pTerrainModel->Initialize(terrainSettings);
      }
      else if (argc >= 2 
         && argv[0].IsString()
         && IsNumber(argv[1]))
      {
         terrainSettings.SetHeightData(kTHD_HeightMap);
         terrainSettings.SetHeightMap(argv[0]);
         terrainSettings.SetHeightMapScale(static_cast<float>(argv[1]) * 255);
         pTerrainModel->Initialize(terrainSettings);
      }
      else
      {
         ErrorMsg("Invalid parameters to SetTerrain\n");
      }
   }
   
   return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUISoundPlayer
//

class cGUISoundPlayer : public cComObject<IMPLEMENTS(IGUIEventListener)>
{
public:
   cGUISoundPlayer();
   ~cGUISoundPlayer();

   virtual tResult OnEvent(IGUIEvent * pEvent);

private:
   void PlaySound(const tChar * pszSound);
};

////////////////////////////////////////

cGUISoundPlayer::cGUISoundPlayer()
{
}

////////////////////////////////////////

cGUISoundPlayer::~cGUISoundPlayer()
{
}

////////////////////////////////////////

tResult cGUISoundPlayer::OnEvent(IGUIEvent * pEvent)
{
   tGUIEventCode eventCode;
   if (pEvent != NULL && pEvent->GetEventCode(&eventCode) == S_OK)
   {
      // TODO: fix hard-coded sound names
      if (eventCode == kGUIEventClick)
      {
         PlaySound("click.wav");
      }
      else if (eventCode == kGUIEventMouseEnter)
      {
         cAutoIPtr<IGUIElement> pElement;
         cAutoIPtr<IGUIButtonElement> pButton;
         if (pEvent->GetSourceElement(&pElement) == S_OK
            && pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButton) == S_OK)
         {
            PlaySound("bonk.wav");
         }
      }
   }

   return S_OK;
}

////////////////////////////////////////

void cGUISoundPlayer::PlaySound(const tChar * pszSound)
{
   tSoundId soundId;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszSound, kRT_WavSound, NULL, (void**)&soundId) == S_OK)
   {
      UseGlobal(SoundManager);
      pSoundManager->Play(soundId);
   }
}


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   CameraCreate();
   InputCreate();
   SimCreate();
   ResourceManagerCreate();
   ScriptInterpreterCreate();
   GUIContextCreate();
   GUIFactoriesCreate();
   GUIFontFactoryCreate();
   GUIEventSoundsCreate();
   EntityManagerCreate();
   ThreadCallerCreate();
   SaveLoadManagerCreate();
   TerrainModelCreate();
   TerrainRendererCreate();
   SoundManagerCreate();
   RendererCreate();
}

///////////////////////////////////////////////////////////////////////////////

static bool ScriptExecResource(IScriptInterpreter * pInterpreter, const tChar * pszResource)
{
   bool bResult = false;

   char * pszCode = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszResource, kRT_Text, NULL, (void**)&pszCode) == S_OK)
   {
      bResult = SUCCEEDED(pInterpreter->ExecString(pszCode));
   }

   return bResult;
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

static bool MainInit(int argc, tChar * argv[])
{
   if (InitGlobalConfig(argc, argv) != S_OK)
   {
      return false;
   }

   cStr temp;
   if (ConfigGet(_T("debug_log"), &temp) == S_OK)
   {
      techlog.OpenLogFile(cFileSpec(temp.c_str()));
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
   EngineRegisterScriptFunctions();
   TerrainRegisterResourceFormats();
   ImageRegisterResourceFormats();
   HackScenarioRegisterResourceFormat();

   if (ConfigGet(_T("data"), &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddDirectoryTreeFlattened(temp.c_str());
   }

   cStr script(kAutoExecScript);
   ConfigGet(_T("autoexec_script"), &script);
   if (!script.empty())
   {
      UseGlobal(ScriptInterpreter);
      if (pScriptInterpreter->ExecFile(script.c_str()) != S_OK)
      {
         if (!ScriptExecResource(pScriptInterpreter, script.c_str()))
         {
            ErrorMsg1("Start-up script \"%s\" failed to execute or was not found\n", script.c_str());
            return false;
         }
      }
   }

   g_fov = kDefaultFov;
   ConfigGet(_T("fov"), &g_fov);

   int width = kDefaultWidth;
   int height = kDefaultHeight;
   int bpp = kDefaultBpp;
   ConfigGet(_T("screen_width"), &width);
   ConfigGet(_T("screen_height"), &height);
   ConfigGet(_T("screen_bpp"), &bpp);
#ifdef __CYGWIN__
// HACK
   bool bFullScreen = ConfigIsTrue(_T("full_screen"));
#else
   bool bFullScreen = ConfigIsTrue(_T("full_screen")) && !IsDebuggerPresent();
#endif

   if (!SysCreateWindow(_T("Game"), width, height))
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
      ErrorMsg("Failed to get a default font interface pointer for showing frame stats\n");
      return false;
   }

   SysAppActivate(true);

#ifdef HAVE_CPPUNIT
   if (FAILED(SysRunUnitTests()))
   {
      return false;
   }
#endif

   UseGlobal(Camera);
   pCamera->SetPerspective(g_fov, (float)width / height, kZNear, kZFar);

   g_pGameCameraController = new cGameCameraController;
   g_pGameCameraController->Connect();

   UseGlobal(ScriptInterpreter);
   pScriptInterpreter->CallFunction("GameInit");

   UseGlobal(Sim);
   pSim->Go();

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static void MainTerm()
{
   UseGlobal(Sim);
   pSim->Stop();

   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   SafeRelease(g_pFont);

   if (g_pGameCameraController)
   {
      g_pGameCameraController->Disconnect();
   }

   // This will make sure the GL context is destroyed
   SysQuit();

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

static bool MainFrame()
{
   UseGlobal(Sim);
   pSim->NextFrame();

   UseGlobal(Renderer);
   if (pRenderer->BeginScene() != S_OK)
   {
      return false;
   }

   UseGlobal(Camera);
   pCamera->SetGLState();

   UseGlobal(TerrainRenderer);
   pTerrainRenderer->Render();

   UseGlobal(EntityManager);
   pEntityManager->RenderAll();

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
         g_pFont->RenderText(szStats, _tcslen(szStats), &rect, kRT_NoClip | kRT_DropShadow, kDefStatsColor);
      }

      pRenderDeviceContext->End2D();
   }

   pRenderer->EndScene();
   SysSwapBuffers();

   return true;
}

///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) && !defined(__CYGWIN__)
int STDCALL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nShowCmd)
{
   if (!MainInit(__argc, __targv))
   {
      MainTerm();
      return -1;
   }

   int result = SysEventLoop(MainFrame, ResizeHack);

   MainTerm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(__CYGWIN__)
int main(int argc, char * argv[])
{
   if (!MainInit(argc, argv))
   {
      MainTerm();
      return EXIT_FAILURE;
   }

   int result = SysEventLoop(MainFrame, ResizeHack);

   MainTerm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////
