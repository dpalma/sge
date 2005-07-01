///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "groundtiled.h"
#include "script.h"
#include "cameracontroller.h"

#include "guiapi.h"
#include "sceneapi.h"
#include "meshapi.h"
#include "sim.h"
#include "inputapi.h"
#include "engineapi.h"
#include "entityapi.h"
#include "scriptvar.h"
#include "sys.h"

#include "renderapi.h"

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

#include "dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' used in base member initializer list

// The following definitions are required for WinMain
F_DECLARE_HANDLE(HINSTANCE);
typedef char * LPSTR;

///////////////////////////////////////////////////////////////////////////////

#define kAutoExecScript "autoexec.lua"
#define kDefaultWidth   800
#define kDefaultHeight  600
#define kDefaultBpp     16
#define kDefaultFov     70

const float kZNear = 1;
const float kZFar = 2000;

static const float kGroundScaleY = 0.25f;

static const float kDefStatsX = 25;
static const float kDefStatsY = 25;
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
   GUIContextCreate();
   GUIFactoryCreate();
   EntityManagerCreate();
   ThreadCallerCreate();
}

///////////////////////////////////////////////////////////////////////////////

static bool ScriptExecResource(IScriptInterpreter * pInterpreter, const char * pszResource)
{
   bool bResult = false;

   char * pszCode = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(tResKey(pszResource, kRC_Text), (void**)&pszCode) == S_OK)
   {
      bResult = SUCCEEDED(pInterpreter->ExecString(pszCode));
      pResourceManager->Unload(tResKey(pszResource, kRC_Text));
   }

   return bResult;
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

static tResult InitGlobalConfig(int argc, char * argv[])
{
   Assert(argc > 0);

   cFileSpec cfgFile(argv[0]);
   cfgFile.SetFileExt("cfg");

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(cfgFile);
   if (!pStore)
   {
      return E_OUTOFMEMORY;
   }
   pStore->Load(g_pConfig);

   ParseCommandLine(argc, argv, g_pConfig);

   return S_OK;
}

static bool MainInit(int argc, char * argv[])
{
   if (InitGlobalConfig(argc, argv) != S_OK)
   {
      return false;
   }

   cStr temp;
   if (ConfigGet("debug_log", &temp) == S_OK)
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

   TargaFormatRegister();
   BmpFormatRegister();
   TextFormatRegister("txt");
   TextFormatRegister("lua");
   TextFormatRegister("xml");
   Ms3dFormatRegister();
   GlTextureResourceRegister();
   EngineRegisterResourceFormats();

   if (ConfigGet("data", &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddDirectoryTreeFlattened(temp.c_str());
   }

   cStr script(kAutoExecScript);
   ConfigGet("autoexec_script", &script);
   if (!script.empty())
   {
      UseGlobal(ScriptInterpreter);
      if (pScriptInterpreter->ExecFile(script.c_str()) != S_OK)
      {
         if (!ScriptExecResource(pScriptInterpreter, script.c_str()))
         {
            return false;
         }
      }
   }

   g_fov = kDefaultFov;
   ConfigGet("fov", &g_fov);

   int width = kDefaultWidth;
   int height = kDefaultHeight;
   int bpp = kDefaultBpp;
   ConfigGet("screen_width", &width);
   ConfigGet("screen_height", &height);
   ConfigGet("screen_bpp", &bpp);
#ifdef __CYGWIN__
// HACK
   bool bFullScreen = ConfigIsTrue("full_screen");
#else
   bool bFullScreen = ConfigIsTrue("full_screen") && !IsDebuggerPresent();
#endif

   if (!SysCreateWindow("Game", width, height))
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

   if (FAILED(GUIFontGetDefault(&g_pFont)))
   {
      WarnMsg("Failed to get a default font interface pointer for showing frame stats\n");
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
   if (!RunUnitTests())
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

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

static void MainFrame()
{
   Assert(g_pRenderDevice != NULL);

   UseGlobal(Sim);
   pSim->NextFrame();

   g_pRenderDevice->BeginScene();

   UseGlobal(Scene);
   pScene->Render(g_pRenderDevice);

   GlBegin2D();

   UseGlobal(GUIContext);
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

   GlEnd2D();

   g_pRenderDevice->EndScene();
   SysSwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) && !defined(__CYGWIN__)
int STDCALL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nShowCmd)
{
   if (!MainInit(__argc, __argv))
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
