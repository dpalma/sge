///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "groundtiled.h"
#include "script.h"
#include "cameracontroller.h"

#include "guiapi.h"
#include "sceneapi.h"
#include "sim.h"
#include "inputapi.h"
#include "engineapi.h"
#include "entityapi.h"
#include "saveloadapi.h"
#include "scriptvar.h"
#include "sys.h"
#include "terrainapi.h"

#include "renderapi.h"

#include "resourceapi.h"
#include "configapi.h"
#include "filespec.h"
#include "filepath.h"
#include "techstring.h"
#include "globalobj.h"
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

///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<cGameCameraController> g_pGameCameraController;

cAutoIPtr<ISceneCamera> g_pGameCamera;

cAutoIPtr<cTerrainNode> g_pTerrainRoot;

cAutoIPtr<IGUIFont> g_pFont;

float g_fov;

cAutoIPtr<IRenderDevice> g_pRenderDevice;

///////////////////////////////////////////////////////////////////////////////

class cTiledGroundLocator : public cTerrainLocatorHack
{
public:
   virtual void Locate(float nx, float nz, float * px, float * py, float * pz)
   {
      if (g_pTerrainRoot != NULL)
      {
         tVec2 groundDims = g_pTerrainRoot->GetDimensions();
         *px = nx * groundDims.x;
         *py = g_pTerrainRoot->GetElevation(nx, nz);
         *pz = nz * groundDims.y;
      }
   }
};

cTiledGroundLocator g_terrainLocator;

///////////////////////////////////////////////////////////////////////////////

IRenderDevice * AccessRenderDevice()
{
   return static_cast<IRenderDevice *>(g_pRenderDevice);
}


///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(ViewSetPos)
{
   if (argc == 2 && argv[0].IsNumber() && argv[1].IsNumber())
   {
      float x = argv[0];
      float z = argv[1];

      if (x >= 0 && x <= 1 && z >= 0 && z <= 1)
      {
         if (g_pTerrainRoot != NULL)
         {
            tVec2 groundDims = g_pTerrainRoot->GetDimensions();

            x *= groundDims.x;
            z *= groundDims.y;

            g_pGameCameraController->LookAtPoint(x, z);
         }
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
   if (g_pGameCamera != NULL)
   {
      g_pGameCamera->SetPerspective(g_fov, static_cast<float>(width) / height, kZNear, kZFar);
   }
}


/////////////////////////////////////////////////////////////////////////////////
////
//// CLASS: cSpinner
////
//
//class cSpinner : public cComObject<IMPLEMENTS(ISimClient)>
//{
//   cSpinner(const cSpinner &);
//   const cSpinner & operator =(const cSpinner &);
//
//public:
//   cSpinner(cSceneNode * pGroup, float degreesPerSec);
//   ~cSpinner();
//
//   virtual void OnFrame(double elapsedTime);
//
//   virtual void DeleteThis() {}
//
//private:
//   cSceneNode * m_pNode;
//   float m_radiansPerSec;
//};
//
/////////////////////////////////////////
//
//cSpinner::cSpinner(cSceneNode * pGroup, float degreesPerSec)
// : m_pNode(pGroup),
//   m_radiansPerSec(Deg2Rad(degreesPerSec))
//{
//   Assert(pGroup != NULL);
//
//   UseGlobal(Sim);
//   pSim->Connect(this);
//}
//
/////////////////////////////////////////
//
//cSpinner::~cSpinner()
//{
//   UseGlobal(Sim);
//   pSim->Disconnect(this);
//
//   m_pNode = NULL;
//}
//
/////////////////////////////////////////
//
//void cSpinner::OnFrame(double elapsedTime)
//{
//   Assert(m_pNode != NULL);
//   tQuat q = QuatFromEulerAngles(tVec3(0, m_radiansPerSec * elapsedTime, 0));
//   m_pNode->SetLocalRotation(m_pNode->GetLocalRotation() * q);
//}


///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(SetTerrain)
{
   UseGlobal(TerrainModel);

   if (argc == 1 && argv[0].IsString())
   {
      g_pTerrainRoot = TerrainNodeCreate(AccessRenderDevice(), argv[0], kGroundScaleY, NULL);
   }
   else if (argc == 2 
      && argv[0].IsString()
      && argv[1].IsNumber())
   {
      g_pTerrainRoot = TerrainNodeCreate(AccessRenderDevice(), argv[0], argv[1], NULL);
   }
   else if (argc == 3 
      && argv[0].IsString()
      && argv[1].IsNumber()
      && argv[2].IsString())
   {
      g_pTerrainRoot = TerrainNodeCreate(AccessRenderDevice(), argv[0], argv[1], argv[2]);

      cTerrainSettings terrainSettings;
      terrainSettings.SetTileSet("defaulttiles.xml"); // HACK TODO
      terrainSettings.SetTileSize(16);
      terrainSettings.SetTileCountX(64);
      terrainSettings.SetTileCountZ(64);
      terrainSettings.SetHeightData(kTHD_HeightMap);
      terrainSettings.SetHeightMap(argv[0]);
      terrainSettings.SetHeightMapScale(static_cast<float>(argv[1]) * 255);
      pTerrainModel->Initialize(terrainSettings);
   }
   else
   {
      DebugMsg("Warning: Invalid parameters to SetTerrain\n");
   }
   
   if (g_pTerrainRoot)
   {
      UseGlobal(Scene);
      pScene->AddEntity(kSL_Terrain, g_pTerrainRoot);
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   InputCreate();
   SimCreate();
   ResourceManagerCreate();
   SceneCreate();
   ScriptInterpreterCreate();
   GUIContextCreate();
   GUIFactoryCreate();
   GUIFontFactoryCreate();
   EntityManagerCreate();
   ThreadCallerCreate();
   SaveLoadManagerCreate();
   TerrainModelCreate();
   TerrainRendererCreate();
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
      DebugEchoFileStart(temp.c_str());
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

   if (RenderDeviceCreate(&g_pRenderDevice) != S_OK)
   {
      return false;
   }

   UseGlobal(ThreadCaller);
   if (FAILED(pThreadCaller->ThreadInit()))
   {
      return false;
   }

   UseGlobal(EntityManager);
   pEntityManager->SetTerrainLocatorHack(&g_terrainLocator);
   pEntityManager->SetRenderDeviceHack(g_pRenderDevice);

   UseGlobal(GUIContext);
   if (FAILED(pGUIContext->GetDefaultFont(&g_pFont)))
   {
      ErrorMsg("Failed to get a default font interface pointer for showing frame stats\n");
      return false;
   }

   SysAppActivate(true);

   g_pGameCamera = SceneCameraCreate();
   g_pGameCamera->SetPerspective(g_fov, (float)width / height, kZNear, kZFar);

   g_pGameCameraController = new cGameCameraController(g_pGameCamera);
   g_pGameCameraController->Connect();

   UseGlobal(Scene);
   pScene->SetCamera(kSL_Terrain, g_pGameCamera);

   UseGlobal(ScriptInterpreter);
   pScriptInterpreter->CallFunction("GameInit");

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

static void MainTerm()
{
   UseGlobal(Sim);
   pSim->Stop();

   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   SafeRelease(g_pFont);

   SafeRelease(g_pRenderDevice);

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
   Assert(g_pRenderDevice != NULL);

   UseGlobal(Sim);
   pSim->NextFrame();

   g_pRenderDevice->BeginScene();

   UseGlobal(Scene);
   pScene->Render(g_pRenderDevice);

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

   g_pRenderDevice->EndScene();
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
