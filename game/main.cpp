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
#include "raycast.h"
#include "mesh.h"
#include "scenenode.h"
#include "scenecamera.h"
#include "scenemesh.h"
#include "camera.h"
#include "script.h"
#include "scriptvm.h"

#include "font.h"
#include "image.h"
#include "render.h"

#include "techmath.h"
#include "techtime.h"
#include "window.h"
#include "resmgr.h"
#include "configapi.h"
#include "filespec.h"
#include "filepath.h"
#include "matrix4.h"
#include "vec2.h"
#include "vec4.h"
#include "str.h"
#include "keys.h"

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

///////////////////////////////////////////////////////////////////////////////

#define kAutoExecScript "autoexec.lua"
#define kDefaultWidth   800
#define kDefaultHeight  600
#define kDefaultBpp     16
#define kDefaultFov     70

const float kZNear = 1;
const float kZFar = 2000;

static const float kRotateDegreesPerSec = 20;

static const char kszSelIndicatorMesh[] = "arrow.ms3d";

///////////////////////////////////////////////////////////////////////////////

cSceneCameraGroup * g_pGameCamera = NULL;
cSceneCameraGroup * g_pUICamera = NULL;

double g_fov;

cAutoIPtr<IUIManager> g_pUIManager;

cAutoIPtr<IRenderDevice> g_pRenderDevice;
cAutoIPtr<IWindow> g_pWindow;

cAutoIPtr<IResourceManager> g_pResourceManager;

cSim g_sim;

///////////////////////////////////////////////////////////////////////////////

IRenderDevice * AccessRenderDevice()
{
   return static_cast<IRenderDevice *>(g_pRenderDevice);
}

IResourceManager * AccessResourceManager()
{
   return static_cast<IResourceManager *>(g_pResourceManager);
}

///////////////////////////////////////////////////////////////////////////////

class cPickNodeVisitor : public cSceneNodeVisitor
{
public:
   cPickNodeVisitor(const tVec3 & rayDir, const tVec3 & rayOrigin);

   virtual void VisitSceneNode(cSceneNode * pNode);

   tVec3 m_rayDir, m_rayOrigin;
   std::vector<cSceneNode *> m_hitNodes;
};

cPickNodeVisitor::cPickNodeVisitor(const tVec3 & rayDir, const tVec3 & rayOrigin)
 : m_rayDir(rayDir),
   m_rayOrigin(rayOrigin)
{
}

void cPickNodeVisitor::VisitSceneNode(cSceneNode * pNode)
{
   Assert(pNode != NULL);

   if (pNode->IsPickable())
   {
      const cBoundingVolume * pBounds = pNode->GetBoundingVolume();
      if (pBounds != NULL)
      {
         if (pBounds->Intersects(m_rayOrigin, m_rayDir))
         {
            pNode->Hit();
            m_hitNodes.push_back(pNode);
         }
         else
         {
            pNode->ClearHitState();
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

static bool BuildPickRay(int x, int y, tVec3 * pRay)
{
   Assert(g_pGameCamera != NULL);

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   y = viewport[3] - y;

   // convert screen coords to normalized (origin at center, [-1..1])
   float normx = (float)(x - viewport[0]) * 2.f / viewport[2] - 1.f;
   float normy = (float)(y - viewport[1]) * 2.f / viewport[3] - 1.f;

   Assert(g_pGameCamera != NULL);
   const sMatrix4 & m = g_pGameCamera->GetModelViewProjectionInverseMatrix();

   tVec4 n = m.Transform(tVec4(normx, normy, -1, 1));
   if (n.w == 0.0f)
      return false;
   n.x /= n.w;
   n.y /= n.w;
   n.z /= n.w;

   tVec4 f = m.Transform(tVec4(normx, normy, 1, 1));
   if (f.w == 0.0f)
      return false;
   f.x /= f.w;
   f.y /= f.w;
   f.z /= f.w;

   Assert(pRay != NULL);
   *pRay = tVec3(f.x - n.x, f.y - n.y, f.z - n.z);
   pRay->Normalize();

   return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGameCameraController
//

class cGameCameraController : public cComObject2<IMPLEMENTS(ISimClient), IMPLEMENTS(IInputListener)>
{
   cGameCameraController(const cGameCameraController &);
   const cGameCameraController & operator =(const cGameCameraController &);

public:
   cGameCameraController(cSceneCameraGroup * pCamera);
   ~cGameCameraController();

   void Connect();
   void Disconnect();

   virtual void OnFrame(double elapsedTime);

   virtual bool OnMouseEvent(int x, int y, uint mouseState, double time);
   virtual bool OnKeyEvent(long key, bool down, double time);

   void LookAtPoint(float x, float z);

   const tVec3 & GetEyePosition() const { return m_eye; }

private:
   enum eConstants
   {
      kDefaultElevation = 100,
      kDefaultPitch = 70,
      kDefaultSpeed = 50,
   };

   float m_pitch, m_oneOverTangentPitch, m_elevation;
   tVec3 m_eye, m_focus, m_velocity;

   sMatrix4 m_rotation;

   cSceneCameraGroup * m_pCamera;
};

cAutoIPtr<cGameCameraController> g_pGameCameraController;

///////////////////////////////////////

cGameCameraController::cGameCameraController(cSceneCameraGroup * pCamera)
 : m_pitch(kDefaultPitch),
   m_oneOverTangentPitch(0),
   m_elevation(kDefaultElevation),
   m_focus(0,0,0),
   m_velocity(0,0,0),
   m_pCamera(pCamera)
{
   ConfigGet("view_elevation", &m_elevation);
   ConfigGet("view_pitch", &m_pitch);
   MatrixRotateX(m_pitch, &m_rotation);
   m_oneOverTangentPitch = 1.0f / tanf(m_pitch);
}

///////////////////////////////////////

cGameCameraController::~cGameCameraController()
{
}

///////////////////////////////////////

void cGameCameraController::Connect()
{
   g_sim.Connect(this);
   InputAddListener(this);
}

///////////////////////////////////////

void cGameCameraController::Disconnect()
{
   InputRemoveListener(this);
   g_sim.Disconnect(this);
}

///////////////////////////////////////

void cGameCameraController::OnFrame(double elapsedTime)
{
   m_focus += m_velocity * (float)elapsedTime;

   float zOffset = m_elevation * m_oneOverTangentPitch;

   m_eye = tVec3(m_focus.x, m_focus.y + m_elevation, m_focus.z + zOffset);

   // Very simple third-person camera model. Always looking down the -z axis
   // and slightly pitched over the x axis.
   sMatrix4 mt;
   MatrixTranslate(-m_eye.x, -m_eye.y, -m_eye.z, &mt);

   sMatrix4 newModelView = m_rotation * mt;
   m_pCamera->SetModelViewMatrix(newModelView);
}

///////////////////////////////////////

bool cGameCameraController::OnMouseEvent(int x, int y, uint mouseState, double time)
{
   if (mouseState & kLMouseDown)
   {
      tVec3 dir;
      if (BuildPickRay(x, y, &dir))
      {
         cPickNodeVisitor pickVisitor(dir, GetEyePosition());
         g_pGameCamera->Traverse(&pickVisitor);

         if (pickVisitor.m_hitNodes.empty())
         {
            tVec3 intersect;
            if (RayIntersectPlane(GetEyePosition(), dir, tVec3(0,1,0), 0, &intersect))
            {
               DebugMsg3("Hit the ground at approximately (%.1f,%.1f,%.1f)\n",
                  intersect.x, intersect.y, intersect.z);
            }
         }
      }

      return true;
   }

   return false;
}

///////////////////////////////////////

bool cGameCameraController::OnKeyEvent(long key, bool down, double time)
{
   bool bUpdateCamera = false;

   switch (key)
   {
      case kLeft:
      {
         m_velocity.x = down ? -kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kRight:
      {
         m_velocity.x = down ? kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kUp:
      {
         m_velocity.z = down ? -kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kDown:
      {
         m_velocity.z = down ? kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kMouseWheelUp:
      {
         m_elevation++;
         bUpdateCamera = true;
         break;
      }

      case kMouseWheelDown:
      {
         m_elevation--;
         bUpdateCamera = true;
         break;
      }
   }

   return bUpdateCamera;
}

///////////////////////////////////////

void cGameCameraController::LookAtPoint(float x, float z)
{
   m_focus = tVec3(x, 0, z);
}

///////////////////////////////////////

void ViewGetPos(tVec3 * pPos)
{
   if (pPos != NULL)
   {
      *pPos = g_pGameCameraController->GetEyePosition();
   }
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
         tVec2 groundDims = g_terrainData.GetDimensions();

         x *= groundDims.x;
         z *= groundDims.y;

         g_pGameCameraController->LookAtPoint(x, z);
      }
      else
      {
         DebugMsg2("ViewSetPos arguments %f, %f, out of range\n", x, z);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

class cWindowSink : public cComObject<IMPLEMENTS(IWindowSink)>
{
public:
   virtual void OnKeyEvent(long key, bool down, double time);
   virtual void OnMouseEvent(int x, int y, uint mouseState, double time);
   virtual void OnDestroy(double time);
   virtual void OnResize(int width, int height, double time);
   virtual void OnActivateApp(bool bActive, double time);
};

void cWindowSink::OnKeyEvent(long key, bool down, double time)
{
   KeyEvent(key, down, time);
}

void cWindowSink::OnMouseEvent(int x, int y, uint mouseState, double time)
{
   MouseEvent(x, y, mouseState, time);
}

void cWindowSink::OnDestroy(double time)
{
   SysQuit();
}

void cWindowSink::OnResize(int width, int height, double time)
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

void cWindowSink::OnActivateApp(bool bActive, double time)
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
// CLASS: cSelectionIndicatorNode
//

class cSelectionIndicatorNode : public cSceneMesh
{
public:
   cSelectionIndicatorNode();

   virtual void Update(float timeDelta);

private:
   cAutoIPtr<IMesh> m_pMesh;
};

cSelectionIndicatorNode::cSelectionIndicatorNode()
{
   SetPickable(false);
   // TODO: SetTranslation(tVec3(0, parentY, 0));
}

void cSelectionIndicatorNode::Update(float timeDelta)
{
   tQuat q = QuatFromEulerAngles(tVec3(0, Deg2Rad(2 * kRotateDegreesPerSec * timeDelta), 0));
   SetRotation(GetRotation() * q);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSimpleSceneNode
//

class cSimpleSceneNode : public cSceneMesh
{
public:
   cSimpleSceneNode();

   virtual void Update(float timeDelta);

   virtual void Hit();
   virtual void ClearHitState();

private:
   cSelectionIndicatorNode * m_pSel;
};

cSimpleSceneNode::cSimpleSceneNode()
 : m_pSel(NULL)
{
}

void cSimpleSceneNode::Update(float timeDelta)
{
   tQuat q = QuatFromEulerAngles(tVec3(0, Deg2Rad(kRotateDegreesPerSec * timeDelta), 0));
   SetRotation(GetRotation() * q);
}

void cSimpleSceneNode::Hit()
{
   if (m_pSel == NULL)
   {
      m_pSel = new cSelectionIndicatorNode;
      AddChild(m_pSel);
      m_pSel->SetMesh(kszSelIndicatorMesh);
   }
   else
   {
      RemoveChild(m_pSel);
      delete m_pSel, m_pSel = NULL;
   }
}

void cSimpleSceneNode::ClearHitState()
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
         float y = g_terrainData.GetElevation(Round(x), Round(z));

         tVec2 groundDims = g_terrainData.GetDimensions();

         x *= groundDims.x;
         z *= groundDims.y;

         cSimpleSceneNode * pNode = new cSimpleSceneNode;

         pNode->SetMesh(ScriptArgAsString(0));
         pNode->SetTranslation(tVec3(x,y,z));

         Assert(g_pGameCamera != NULL);
         g_pGameCamera->AddChild(pNode);
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

   virtual void Render();
};

cUIManagerSceneNode::cUIManagerSceneNode()
{
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

   char szStats[30];
   sprintf(szStats, "%.2f FPS\n", CalcFramesPerSec());
   UIDrawText(cUIPoint(50, 50), szStats);
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

bool MainInit(int argc, char * argv[])
{
   Assert(argc > 0);

   cFileSpec file(argv[0]);
   file.SetPath(cFilePath());
   file.SetFileExt("cfg");

   cAutoIPtr<IConfigStore> pConfigStore = CreateTextConfigStore(file);
   pConfigStore->Load(g_pConfig);

   g_pConfig->ParseCmdLine(argc, argv);

   g_pResourceManager = ResourceManagerCreate();

   cStr temp;
   if (ConfigGet("data", &temp) == S_OK)
   {
      AccessResourceManager()->AddSearchPath(temp, true);
   }

   if (ConfigGet("debug_log", &temp) == S_OK)
   {
      DebugEchoFileStart(temp);
   }

   srand(time(NULL));
   SeedRand(time(NULL));

   ScriptInit();

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

   cWindowSink * pSink = new cWindowSink;
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

   g_pGameCamera->AddChild(TerrainRootNodeCreate());

   g_pGameCameraController = new cGameCameraController(g_pGameCamera);
   g_pGameCameraController->Connect();

   g_pUICamera = new cSceneCameraGroup;
   g_pUICamera->SetOrtho(0, width, height, 0, -99999, 99999);

   g_pUIManager = UIManagerCreate();
   g_pUICamera->AddChild(new cUIManagerSceneNode);

   if (ConfigGet("terrain", &temp) == S_OK)
   {
      g_terrainData.LoadHeightMap(temp);
   }
   else
   {
      DebugMsg("No terrain data\n");
      return false;
   }

   ScriptCallFunction("GameInit");

   g_sim.Go();

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
   g_sim.Stop();
   SafeRelease(g_pUIManager);
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

   SafeRelease(g_pResourceManager);

   g_pGameCameraController->Disconnect();

   delete g_pGameCamera, g_pGameCamera = NULL;

   delete g_pUICamera, g_pUICamera = NULL;
}

///////////////////////////////////////////////////////////////////////////////

class cUpdateVisitor : public cSceneNodeVisitor
{
public:
   virtual void VisitSceneNode(cSceneNode * pNode);
};

void cUpdateVisitor::VisitSceneNode(cSceneNode * pNode)
{
   Assert(pNode != NULL);
   pNode->Update(g_sim.GetFrameTime());
}

///////////////////////////////////////////////////////////////////////////////

void MainFrame()
{
   Assert(g_pRenderDevice != NULL);

   g_sim.NextFrame();

   cUpdateVisitor updateVisitor;
   g_pGameCamera->Traverse(&updateVisitor);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   g_pGameCamera->Render();

   g_pUICamera->Render();

   g_pWindow->SwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////
