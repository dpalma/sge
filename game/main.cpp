///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ggl.h"
#include "gcommon.h"
#include "uimgr.h"
#include "uirender.h"
#include "sim.h"
#include "groundtiled.h"
#include "input.h"
#include "sys.h"
#include "mesh.h"
#include "scenenode.h"
#include "scenecamera.h"
#include "scenemesh.h"
#include "script.h"
#include "scriptvm.h"
#include "cameracontroller.h"

#include "render.h"
#include "font.h"

#include "techmath.h"
#include "techtime.h"
#include "window.h"
#include "resmgr.h"
#include "configapi.h"
#include "filespec.h"
#include "filepath.h"
#include "matrix4.h"
#include "str.h"
#include "globalobj.h"

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

static const float kRotateDegreesPerSec = 20;

static const char kszSelIndicatorMesh[] = "arrow.ms3d";

///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<cGameCameraController> g_pGameCameraController;

cSceneCameraGroup * g_pGameCamera = NULL;
cSceneCameraGroup * g_pUICamera = NULL;

cTerrainNode * g_pTerrainRoot = NULL;

double g_fov;

cAutoIPtr<IUIManager> g_pUIManager;

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
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cWindowInputBroker
//
// Receives event notifications from the Windows or X window and channels
// them into the game input system.

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
   KeyEvent(key, down, time);
}

void cWindowInputBroker::OnMouseEvent(int x, int y, uint mouseState, double time)
{
   MouseEvent(x, y, mouseState, time);
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
}

///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(LogEnableChannel)
{
   if (ScriptArgc() == 1 && ScriptArgIsString(0))
   {
      LogEnableChannel(ScriptArgAsString(0), true);
   }
   else if (ScriptArgc() == 2 && ScriptArgIsString(0) && ScriptArgIsNumber(1))
   {
      LogEnableChannel(ScriptArgAsString(0), ScriptArgAsNumber(1) ? true : false);
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSpinner
//

class cSpinner : public cComObject<IMPLEMENTS(ISimClient)>
{
   cSpinner(const cSpinner &);
   const cSpinner & operator =(const cSpinner &);

public:
   cSpinner(cSceneTransformGroup * pGroup, float degreesPerSec);
   ~cSpinner();

   void Connect();
   void Disconnect();

   virtual void OnFrame(double elapsedTime);

   virtual void DeleteThis() {}

private:
   cSceneTransformGroup * m_pGroup;
   float m_radiansPerSec;
};

///////////////////////////////////////

cSpinner::cSpinner(cSceneTransformGroup * pGroup, float degreesPerSec)
 : m_pGroup(pGroup),
   m_radiansPerSec(Deg2Rad(degreesPerSec))
{
   Assert(pGroup != NULL);

   UseGlobal(Sim);
   pSim->Connect(this);
}

///////////////////////////////////////

cSpinner::~cSpinner()
{
   UseGlobal(Sim);
   pSim->Disconnect(this);

   m_pGroup = NULL;
}

///////////////////////////////////////

void cSpinner::OnFrame(double elapsedTime)
{
   Assert(m_pGroup != NULL);
   tQuat q = QuatFromEulerAngles(tVec3(0, m_radiansPerSec * elapsedTime, 0));
   m_pGroup->SetRotation(m_pGroup->GetRotation() * q);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSelectionIndicatorNode
//

class cSelectionIndicatorNode : public cSceneMesh
{
public:
   cSelectionIndicatorNode();
   ~cSelectionIndicatorNode();

private:
   cAutoIPtr<IMesh> m_pMesh;
   cSpinner * m_pSpinner;
};

cSelectionIndicatorNode::cSelectionIndicatorNode()
 : m_pSpinner(new cSpinner(this, kRotateDegreesPerSec))
{
   SetPickable(false);
   // TODO: SetTranslation(tVec3(0, parentY, 0));
}

cSelectionIndicatorNode::~cSelectionIndicatorNode()
{
   delete m_pSpinner, m_pSpinner = NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSelectableSceneNode
//

class cSelectableSceneNode : public cSceneMesh
{
public:
   cSelectableSceneNode();
   ~cSelectableSceneNode();

   virtual void Hit();
   virtual void ClearHitState();

private:
   cSelectionIndicatorNode * m_pSel;
   cSpinner * m_pSpinner;
};

cSelectableSceneNode::cSelectableSceneNode()
 : m_pSel(NULL),
   m_pSpinner(new cSpinner(this, kRotateDegreesPerSec))
{
}

cSelectableSceneNode::~cSelectableSceneNode()
{
   delete m_pSpinner, m_pSpinner = NULL;
}

void cSelectableSceneNode::Hit()
{
   if (m_pSel == NULL)
   {
      m_pSel = new cSelectionIndicatorNode;
      m_pSel->SetMesh(kszSelIndicatorMesh);
      AddChild(m_pSel);
   }
   else
   {
      ClearHitState();
   }
}

void cSelectableSceneNode::ClearHitState()
{
   if (m_pSel != NULL)
   {
      RemoveChild(m_pSel);
      delete m_pSel, m_pSel = NULL;
   }
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

            cSelectableSceneNode * pNode = new cSelectableSceneNode;

            pNode->SetMesh(ScriptArgAsString(0));
            pNode->SetTranslation(tVec3(x,y,z));

            Assert(g_pGameCamera != NULL);
            g_pGameCamera->AddChild(pNode);
         }
      }
      else
      {
         DebugMsg2("EntitySpawnTest arguments %f, %f, out of range\n", x, z);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////

SCRIPT_DEFINE_FUNCTION(quit)
{
   SysQuit();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIManagerSceneNode
//

class cUIManagerSceneNode : public cSceneNode
{
public:
   cUIManagerSceneNode();
   virtual ~cUIManagerSceneNode();

   virtual void Render();

private:
   cAutoIPtr<IRenderFont> m_pFont;
};

cUIManagerSceneNode::cUIManagerSceneNode()
 : m_pFont(FontCreateDefault())
{
   g_pUIManager = UIManagerCreate();
}

cUIManagerSceneNode::~cUIManagerSceneNode()
{
   SafeRelease(g_pUIManager);
}

void cUIManagerSceneNode::Render()
{
   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_DEPTH_TEST);

   if (g_pUIManager != NULL)
   {
      g_pUIManager->Render();
   }

   glPopAttrib();

   if (m_pFont != NULL)
   {
      char szStats[30];
      sprintf(szStats, "%.2f FPS\n", CalcFramesPerSec());
      m_pFont->DrawText(50, 50, szStats, strlen(szStats));
   }
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
   SimCreate();
   ResourceManagerCreate();
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

   ScriptInit();

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

   if (FAILED(g_pWindow->Create(width, height, bpp)))
   {
      return false;
   }

   if (RenderDeviceCreate(&g_pRenderDevice) != S_OK)
      return false;

   cAutoIPtr<IWindowFullScreen> pWindowFullScreen;
   bool bFullScreen = (ConfigIsTrue("full_screen") && !IsDebuggerPresent());
   if (bFullScreen
      && g_pWindow->QueryInterface(IID_IWindowFullScreen, (void * *)&pWindowFullScreen) == S_OK)
   {
      pWindowFullScreen->BeginFullScreen();
   }

   // call init functions after the autoexec script is loaded so they
   // may reference constants defined in the script, and after the 
   // display is created so that there is a gl context
   InputInit();

   g_pGameCamera = new cSceneCameraGroup;
   g_pGameCamera->SetPerspective(g_fov, (float)width / height, kZNear, kZFar);

   if (ConfigGet("terrain", &temp) == S_OK)
   {
      g_pTerrainRoot = TerrainNodeCreate(temp, kGroundScaleY);
      g_pGameCamera->AddChild(g_pTerrainRoot);
   }
   else
   {
      DebugMsg("No terrain data\n");
      return false;
   }

   g_pGameCameraController = new cGameCameraController(g_pGameCamera);
   g_pGameCameraController->Connect();

   g_pUICamera = new cSceneCameraGroup;
   g_pUICamera->SetOrtho(0, width, height, 0, -99999, 99999);

   g_pUICamera->AddChild(new cUIManagerSceneNode);

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

   InputTerm();
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

   delete g_pGameCamera, g_pGameCamera = NULL;

   delete g_pUICamera, g_pUICamera = NULL;

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

void MainFrame()
{
   Assert(g_pRenderDevice != NULL);

   UseGlobal(Sim);
   pSim->NextFrame();

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   g_pGameCamera->Render();

   g_pUICamera->Render();

   g_pWindow->SwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////
