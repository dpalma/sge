///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "groundtiled.h"
#include "sys.h"
#include "script.h"
#include "cameracontroller.h"
#include "entityapi.h"

#include "guiapi.h"
#include "sceneapi.h"
#include "meshapi.h"
#include "sim.h"
#include "inputapi.h"
#include "engineapi.h"

#include "render.h"
#include "textureapi.h"

#include "scriptvar.h"
#include "techmath.h"
#include "window.h"
#include "resmgr.h"
#include "configapi.h"
#include "filespec.h"
#include "filepath.h"
#include "str.h"
#include "globalobj.h"
#include "readwriteapi.h"

#include <ctime>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestFailure.h>
#include <cppunit/SourceLine.h>
#include <cppunit/Exception.h>
#include <cppunit/ui/text/TestRunner.h>
#endif

#include "dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' used in base member initializer list

///////////////////////////////////////////////////////////////////////////////

#define kAutoExecScript "autoexec.lua"
#define kDefaultWidth   800
#define kDefaultHeight  600
#define kDefaultBpp     16
#define kDefaultFov     70

const float kZNear = 1;
const float kZFar = 2000;

static const float kGroundScaleY = 0.25f;

///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<cGameCameraController> g_pGameCameraController;

cAutoIPtr<ISceneCamera> g_pGameCamera;
cAutoIPtr<ISceneCamera> g_pUICamera;

cAutoIPtr<cTerrainNode> g_pTerrainRoot;

double g_fov;

cAutoIPtr<IRenderDevice> g_pRenderDevice;
cAutoIPtr<IWindow> g_pWindow;

///////////////////////////////////////////////////////////////////////////////

IRenderDevice * AccessRenderDevice()
{
   return static_cast<IRenderDevice *>(g_pRenderDevice);
}


///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(ViewSetPos)
{
   if (ScriptArgc() == 2 && ScriptArgIsNumber(0) && ScriptArgIsNumber(1))
   {
      float x = ScriptArgAsNumber(0);
      float z = ScriptArgAsNumber(1);

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
//
// CLASS: cWindowInputBroker
//

class cWindowInputBroker : public cComObject<IMPLEMENTS(IWindowSink)>
{
public:
   virtual void OnKeyEvent(long key, bool down, double time);
   virtual void OnMouseEvent(int x, int y, uint mouseState, double time);
   virtual void OnDestroy(double time);
   virtual void OnResize(int width, int height, double time);
   virtual void OnActivateApp(bool bActive, double time);
};

void cWindowInputBroker::OnKeyEvent(long key, bool down, double time)
{
}

void cWindowInputBroker::OnMouseEvent(int x, int y, uint mouseState, double time)
{
}

void cWindowInputBroker::OnDestroy(double time)
{
   SysQuit();
}

void cWindowInputBroker::OnResize(int width, int height, double time)
{
   if (AccessRenderDevice() != NULL)
   {
      AccessRenderDevice()->SetViewportSize(width, height);
   }

   if (g_pGameCamera != NULL)
   {
      g_pGameCamera->SetPerspective(g_fov, (float)width / height, kZNear, kZFar);
   }

   if (g_pUICamera != NULL)
   {
      g_pUICamera->SetOrtho(0, width, height, 0, -99999, 99999);
   }
}

void cWindowInputBroker::OnActivateApp(bool bActive, double time)
{
   SysAppActivate(bActive);
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
   if (ScriptArgc() == 1 
      && ScriptArgIsString(0))
   {
      g_pTerrainRoot = TerrainNodeCreate(AccessRenderDevice(), ScriptArgAsString(0), kGroundScaleY, NULL);
   }
   else if (ScriptArgc() == 2 
      && ScriptArgIsString(0)
      && ScriptArgIsNumber(1))
   {
      g_pTerrainRoot = TerrainNodeCreate(AccessRenderDevice(), ScriptArgAsString(0), ScriptArgAsNumber(1), NULL);
   }
   else if (ScriptArgc() == 3 
      && ScriptArgIsString(0)
      && ScriptArgIsNumber(1)
      && ScriptArgIsString(2))
   {
      g_pTerrainRoot = TerrainNodeCreate(AccessRenderDevice(), ScriptArgAsString(0), ScriptArgAsNumber(1), ScriptArgAsString(2));
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
   SceneCreate();
   ScriptInterpreterCreate();
   TextureManagerCreate();
   GUIContextCreate();
   GUIFactoryCreate();
   GUIRenderingToolsCreate();
   EntityManagerCreate();
   EngineCreate();
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEngineConfiguration
//

class cEngineConfiguration : public cComObject<IMPLEMENTS(IEngineConfiguration)>
{
public:
   cEngineConfiguration();

   virtual tResult GetStartupScript(cStr * pScript);

   virtual tResult GetPreferredRenderDevice(uint * pPreferDevice);
   virtual tResult GetRenderDeviceParameters(sRenderDeviceParameters * pParams);
};

///////////////////////////////////////

cEngineConfiguration::cEngineConfiguration()
{
}

///////////////////////////////////////

tResult cEngineConfiguration::GetStartupScript(cStr * pScript)
{
   if (pScript == NULL)
   {
      return E_POINTER;
   }

   if (ConfigGet("autoexec_script", pScript) != S_OK)
   {
      *pScript = kAutoExecScript;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cEngineConfiguration::GetPreferredRenderDevice(uint * pPreferDevice)
{
   if (pPreferDevice == NULL)
   {
      return E_POINTER;
   }

   return ConfigIsTrue("use_d3d") ? kRD_Direct3D : kRD_OpenGL;
}

///////////////////////////////////////

tResult cEngineConfiguration::GetRenderDeviceParameters(sRenderDeviceParameters * pParams)
{
   if (pParams == NULL)
   {
      return E_POINTER;
   }

   int width = kDefaultWidth;
   int height = kDefaultHeight;
   int bpp = kDefaultBpp;

   ConfigGet("screen_width", &width);
   ConfigGet("screen_height", &height);
   ConfigGet("screen_bpp", &bpp);

   pParams->width = width;
   pParams->height = height;
   pParams->bpp = bpp;
   pParams->bFullScreen = ConfigIsTrue("full_screen") && !IsDebuggerPresent();
   pParams->options = kRDO_ShowStatistics;
   pParams->pWindow = NULL;

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

static tResult InitGlobalConfig(int argc, char * argv[])
{
   Assert(argc > 0);

   cFileSpec file(argv[0]);
   file.SetPath(cFilePath());
   file.SetFileExt("cfg");

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(file);
   if (!pStore)
   {
      return E_OUTOFMEMORY;
   }
   pStore->Load(g_pConfig);

   ParseCommandLine(argc, argv, g_pConfig);

   return S_OK;
}

bool MainInit(int argc, char * argv[])
{
   if (InitGlobalConfig(argc, argv) != S_OK)
   {
      return false;
   }

   cStr temp;
   if (ConfigGet("debug_log", &temp) == S_OK)
   {
      DebugEchoFileStart(temp);
   }

   srand(time(NULL));
   SeedRand(time(NULL));

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      DebugMsg("One or more application-level services failed to start!\n");
      return false;
   }

   if (ConfigGet("data", &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddSearchPath(temp);
   }

   g_fov = kDefaultFov;
   ConfigGet("fov", (float *)&g_fov); // @HACK: cast to float pointer

   int width = kDefaultWidth;
   int height = kDefaultHeight;

   ConfigGet("screen_width", &width);
   ConfigGet("screen_height", &height);

   cAutoIPtr<IEngineConfiguration> pEngineConfig(new cEngineConfiguration);
   if (!pEngineConfig)
   {
      return false;
   }

   UseGlobal(Engine);
   if (FAILED(pEngine->Startup(pEngineConfig)))
   {
      return false;
   }

   if (FAILED(pEngine->GetRenderDevice(&g_pRenderDevice)))
   {
      return false;
   }

   g_pRenderDevice->GetWindow(&g_pWindow);

   SysAppActivate(true);

   cWindowInputBroker * pSink = new cWindowInputBroker;
   if (pSink != NULL)
   {
      g_pWindow->Connect(pSink);
      SafeRelease(pSink);
   }

   UseGlobal(Input);
   pInput->AddWindow(g_pWindow);

   g_pUICamera = SceneCameraCreate();
   g_pUICamera->SetOrtho(0, width, height, 0, -99999, 99999);

   g_pGameCamera = SceneCameraCreate();
   g_pGameCamera->SetPerspective(g_fov, (float)width / height, kZNear, kZFar);

   g_pGameCameraController = new cGameCameraController(g_pGameCamera);
   g_pGameCameraController->Connect();

   UseGlobal(Scene);
   pScene->SetCamera(kSL_Terrain, g_pGameCamera);
   pScene->SetCamera(kSL_InGameUI, g_pUICamera);

   ScriptCallFunction("GameInit");

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

void MainTerm()
{
   UseGlobal(Sim);
   pSim->Stop();

   SafeRelease(g_pWindow);

   SafeRelease(g_pRenderDevice);

   if (g_pGameCameraController)
   {
      g_pGameCameraController->Disconnect();
   }

   UseGlobal(Engine);
   pEngine->Shutdown();

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

void MainFrame()
{
   Assert(g_pRenderDevice != NULL);

   UseGlobal(Sim);
   pSim->NextFrame();

   g_pRenderDevice->BeginScene();
   g_pRenderDevice->Clear();

   UseGlobal(Scene);
   pScene->Render(g_pRenderDevice);

   UseGlobal(GUIContext);
   pGUIContext->RenderGUI(g_pRenderDevice);

   g_pRenderDevice->EndScene();
}

///////////////////////////////////////////////////////////////////////////////
