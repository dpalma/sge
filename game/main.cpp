///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ggl.h"
#include "uimgr.h"
#include "groundtiled.h"
#include "sys.h"
#include "script.h"
#include "cameracontroller.h"
#include "uirender.h"
#include "guiapi.h"

#include "sceneapi.h"
#include "mesh.h"
#include "sim.h"
#include "inputapi.h"

#include "render.h"
#include "textureapi.h"

#include "scriptvar.h"
#include "techmath.h"
#include "window.h"
#include "resmgr.h"
#include "configapi.h"
#include "filespec.h"
#include "filepath.h"
#include "matrix4.h"
#include "str.h"
#include "globalobj.h"
#include "vec2.h"

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

cAutoIPtr<cUIManager> g_pUIManager;

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
   glViewport(0, 0, width, height);

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

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(ShowModalDialog)
{
   if (ScriptArgc() == 1 && ScriptArgIsString(0))
   {
      g_pUIManager->ShowModalDialog(ScriptArgAsString(0));
   }
   return 0;
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
      g_pTerrainRoot = TerrainNodeCreate(ScriptArgAsString(0), kGroundScaleY, NULL);
   }
   else if (ScriptArgc() == 2 
      && ScriptArgIsString(0)
      && ScriptArgIsNumber(1))
   {
      g_pTerrainRoot = TerrainNodeCreate(ScriptArgAsString(0), ScriptArgAsNumber(1), NULL);
   }
   else if (ScriptArgc() == 3 
      && ScriptArgIsString(0)
      && ScriptArgIsNumber(1)
      && ScriptArgIsString(2))
   {
      g_pTerrainRoot = TerrainNodeCreate(ScriptArgAsString(0), ScriptArgAsNumber(1), ScriptArgAsString(2));
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

SCRIPT_DEFINE_FUNCTION(EntitySpawnTest)
{
   if (ScriptArgc() == 3
      && ScriptArgIsString(0)
      && ScriptArgIsNumber(1)
      && ScriptArgIsNumber(2))
   {
      float x = ScriptArgAsNumber(1);
      float z = ScriptArgAsNumber(2);

      if (x >= 0 && x <= 1 && z >= 0 && z <= 1)
      {
         if (g_pTerrainRoot != NULL)
         {
            float y = g_pTerrainRoot->GetElevation(Round(x), Round(z));

            tVec2 groundDims = g_pTerrainRoot->GetDimensions();

            x *= groundDims.x;
            z *= groundDims.y;

            UseGlobal(ResourceManager);
            cAutoIPtr<IMesh> pMesh = MeshLoad(pResourceManager, AccessRenderDevice(), ScriptArgAsString(0));
            if (!pMesh)
            {
               DebugMsg1("Error loading mesh \"%s\"\n", ScriptArgAsString(0));
               return 0;
            }

            cAutoIPtr<ISceneEntity> pEntity = SceneEntityCreate(pMesh);
            pEntity->SetLocalTranslation(tVec3(x,y,z));

            UseGlobal(Scene);
            pScene->AddEntity(kSL_Object, pEntity);
         }
      }
      else
      {
         DebugMsg2("EntitySpawnTest arguments %f, %f, out of range\n", x, z);
      }
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
      DebugPrintf(NULL, 0, "WARNING: %d UNIT TESTS FAILED!\n", runner.result().testFailuresTotal());
      CppUnit::TestResultCollector::TestFailures::const_iterator iter;
      for (iter = runner.result().failures().begin(); iter != runner.result().failures().end(); iter++)
      {
         DebugPrintf(NULL, 0, "%s(%d) : %s : %s\n",
            (*iter)->sourceLine().fileName().c_str(),
            (*iter)->sourceLine().isValid() ? (*iter)->sourceLine().lineNumber() : -1,
            (*iter)->failedTestName().c_str(),
            (*iter)->thrownException()->what());
      }
      return false;
   }
   else
   {
      DebugPrintf(NULL, 0, "%d unit tests succeeded\n", runner.result().tests().size());
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
   UIRenderingToolsCreate();
   TextureManagerCreate();
   GUIContextCreate();
   GUIFactoryCreate();
}

///////////////////////////////////////////////////////////////////////////////

bool MainInit(int argc, char * argv[])
{
   Assert(argc > 0);

   cFileSpec file(argv[0]);
   file.SetPath(cFilePath());
   file.SetFileExt("cfg");

   cAutoIPtr<IConfigStore> pConfigStore = CreateTextConfigStore(file);
   pConfigStore->Load(g_pConfig);

   g_pConfig->ParseCmdLine(argc, argv);

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

   // Registers functions that were added at static initialization time
   ScriptInit();

   if (ConfigGet("data", &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddSearchPath(temp);
   }

   cStr autoExecScript(kAutoExecScript);
   ConfigGet("autoexec_script", &autoExecScript);

   if (!ScriptExecFile(autoExecScript))
   {
      DebugMsg1("Error parsing or executing %s\n", autoExecScript.c_str());
      return false;
   }

   g_fov = kDefaultFov;
   ConfigGet("fov", (float *)&g_fov); // @HACK: cast to float pointer

   int width = kDefaultWidth;
   int height = kDefaultHeight;
   int bpp = kDefaultBpp;

   ConfigGet("screen_width", &width);
   ConfigGet("screen_height", &height);
   ConfigGet("screen_bpp", &bpp);

   g_pWindow = WindowCreate();

   if (!g_pWindow)
   {
      return false;
   }

   cWindowInputBroker * pSink = new cWindowInputBroker;
   if (pSink != NULL)
   {
      g_pWindow->Connect(pSink);
      SafeRelease(pSink);
   }

   UseGlobal(Input);
   pInput->AddWindow(g_pWindow);

   if (FAILED(g_pWindow->Create(width, height, bpp)))
   {
      return false;
   }

   if (RenderDeviceCreate(kRDO_ShowStatistics, &g_pRenderDevice) != S_OK)
      return false;

   UseGlobal(UIRenderingTools);
   pUIRenderingTools->SetRenderDevice(g_pRenderDevice);

   cAutoIPtr<IWindowFullScreen> pWindowFullScreen;
   bool bFullScreen = (ConfigIsTrue("full_screen") && !IsDebuggerPresent());
   if (bFullScreen
      && g_pWindow->QueryInterface(IID_IWindowFullScreen, (void * *)&pWindowFullScreen) == S_OK)
   {
      pWindowFullScreen->BeginFullScreen();
   }

   g_pUICamera = SceneCameraCreate();
   g_pUICamera->SetOrtho(0, width, height, 0, -99999, 99999);

   g_pUIManager = new cUIManager();

   UseGlobal(Scene);
   pScene->AddEntity(kSL_InGameUI, g_pUIManager);

   g_pGameCamera = SceneCameraCreate();
   g_pGameCamera->SetPerspective(g_fov, (float)width / height, kZNear, kZFar);

   g_pGameCameraController = new cGameCameraController(g_pGameCamera);
   g_pGameCameraController->Connect();

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
   SafeRelease(g_pUIManager);

   UseGlobal(Sim);
   pSim->Stop();

   ScriptTerm();
   SafeRelease(g_pRenderDevice);

   if (g_pWindow)
   {
      cAutoIPtr<IWindowFullScreen> pWindowFullScreen;
      if (g_pWindow->QueryInterface(IID_IWindowFullScreen, (void * *)&pWindowFullScreen) == S_OK)
      {
         pWindowFullScreen->EndFullScreen();
      }

      SafeRelease(g_pWindow);
   }

   if (g_pGameCameraController)
      g_pGameCameraController->Disconnect();

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
   g_pWindow->SwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////
