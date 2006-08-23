///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "cameraapi.h"
#include "guiapi.h"
#include "guielementapi.h"
#include "inputapi.h"
#include "engineapi.h"
#include "entityapi.h"
#include "netapi.h"
#include "renderapi.h"
#include "saveloadapi.h"
#include "schedulerapi.h"
#include "scriptapi.h"
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
#include "statemachine.h"
#include "statemachinetem.h"
#include "threadcallapi.h"
#include "imageapi.h"

#include <ctime>

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


///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<IGUILabelElement> g_pFrameStats;

float g_fov;


///////////////////////////////////////////////////////////////////////////////

void ResizeHack(int width, int height, double time)
{
   UseGlobal(Camera);
   if (!!pCamera)
   {
      pCamera->SetPerspective(g_fov, static_cast<float>(width) / height, kZNear, kZFar);
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: 
//

class cMainRenderTask : public cComObject<IMPLEMENTS(ITask)>
{
public:
   virtual tResult Execute(double time);

private:
};

////////////////////////////////////////

tResult cMainRenderTask::Execute(double time)
{
   if (!!g_pFrameStats)
   {
      tChar szStats[100];
      SysReportFrameStats(szStats, _countof(szStats));

      g_pFrameStats->SetText(szStats);
   }

   UseGlobal(Renderer);
   if (pRenderer->BeginScene() != S_OK)
   {
      return E_FAIL;
   }

   UseGlobal(Camera);
   pCamera->SetGLState();

   UseGlobal(TerrainRenderer);
   pTerrainRenderer->Render();

   UseGlobal(EntityManager);
   pEntityManager->RenderAll();

   UseGlobal(GUIContext);
   pGUIContext->RenderGUI();

   pRenderer->EndScene();
   SysSwapBuffers();

   return S_OK;
}

////////////////////////////////////////

static ITask * MainRenderTaskCreate()
{
   return static_cast<ITask *>(new cMainRenderTask);
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
//
// CLASS: 
//

class cMainInitTask : public cComObject<IMPLEMENTS(ITask)>
                    , public cStateMachine<cMainInitTask, double>
{
public:
   cMainInitTask();

   virtual tResult Execute(double time);

   void OnInitialState(double);
   void OnErrorEnter();
   void OnRunStartupScriptEnter();
   void OnRunUnitTests(double);

private:
   tState m_errorState;
   tState m_runStartupScriptState;
   tState m_runUnitTestsState;
   tState m_finishedState;
};

////////////////////////////////////////

cMainInitTask::cMainInitTask()
 : m_errorState(&cMainInitTask::OnErrorEnter, NULL, NULL)
 , m_runStartupScriptState(&cMainInitTask::OnRunStartupScriptEnter, NULL, NULL)
 , m_runUnitTestsState(NULL, &cMainInitTask::OnRunUnitTests, NULL)
 , m_finishedState(NULL, NULL, NULL)
{
}

////////////////////////////////////////

tResult cMainInitTask::Execute(double time)
{
   Update(time);

   return IsCurrentState(&m_finishedState) ? S_FALSE : S_OK;
}

////////////////////////////////////////

void cMainInitTask::OnInitialState(double time)
{
   GotoState(&m_runStartupScriptState);
}

////////////////////////////////////////

void cMainInitTask::OnErrorEnter()
{
   SysQuit();
}

////////////////////////////////////////

void cMainInitTask::OnRunStartupScriptEnter()
{
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
            GotoState(&m_errorState);
            return;
         }
      }
   }

   UseGlobal(ScriptInterpreter);
   pScriptInterpreter->CallFunction("GameInit");

   // After calling GameInit, try to find a label element for showing frame stats
   {
      UseGlobal(GUIContext);
      cAutoIPtr<IGUIElement> pElement;
      if (pGUIContext->GetOverlayElement(_T("frameStats"), &pElement) == S_OK)
      {
         pElement->QueryInterface(IID_IGUILabelElement, (void**)&g_pFrameStats);
      }
   }

   GotoState(&m_runUnitTestsState);
}

////////////////////////////////////////

void cMainInitTask::OnRunUnitTests(double time)
{
   if (FAILED(SysRunUnitTests()))
   {
      GotoState(&m_errorState);
      return;
   }

   GotoState(&m_finishedState);
}

////////////////////////////////////////

static ITask * MainInitTaskCreate()
{
   return static_cast<ITask *>(new cMainInitTask);
}


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   CameraCreate();
   CameraControlCreate();
   EntityManagerCreate();
   EntityCommandManagerCreate();
   EntityCommandUICreate();
   InputCreate();
   GUIContextCreate();
   GUIFactoryCreate();
   GUIFontCacheCreate();
   GUIEventSoundsCreate();
   NetworkCreate();
   RendererCreate();
   ResourceManagerCreate();
   SaveLoadManagerCreate();
   SchedulerCreate();
   ScriptInterpreterCreate();
   SoundManagerCreate();
   TerrainModelCreate();
   TerrainRendererCreate();
   ThreadCallerCreate();
}


///////////////////////////////////////////////////////////////////////////////

static tResult LoadCfgFile(const tChar * pszArgv0, IDictionary * pDict)
{
   if (pszArgv0 == NULL || pDict == NULL)
   {
      return E_POINTER;
   }

   cFileSpec cfgFile(pszArgv0);
   cfgFile.SetFileExt(_T("cfg"));

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(cfgFile);
   if (!pStore)
   {
      return E_OUTOFMEMORY;
   }

   return pStore->Load(pDict);
}

static tResult LoadIniFile(const tChar * pszArgv0, IDictionary * pDict)
{
   if (pszArgv0 == NULL || pDict == NULL)
   {
      return E_POINTER;
   }

   cFileSpec iniFile(pszArgv0);
   iniFile.SetFileExt(_T("ini"));

   tResult result = E_FAIL;

   cAutoIPtr<IDictionaryStore> pSettingsStore;
   if (DictionaryIniStoreCreate(iniFile, _T("Settings"), &pSettingsStore) == S_OK)
   {
      result = pSettingsStore->Load(pDict);
   }

   cAutoIPtr<IDictionaryStore> pLogChannelsStore;
   if (DictionaryIniStoreCreate(iniFile, _T("LogChannels"), &pLogChannelsStore) == S_OK)
   {
      cAutoIPtr<IDictionary> pLogChannels;
      if (DictionaryCreate(&pLogChannels) == S_OK)
      {
         if (pLogChannelsStore->Load(pLogChannels) == S_OK)
         {
            std::list<cStr> logChannels;
            if (pLogChannels->GetKeys(&logChannels) == S_OK)
            {
               std::list<cStr>::iterator iter = logChannels.begin();
               for (; iter != logChannels.end(); iter++)
               {
                  int enabled = 0;
                  if (pLogChannels->Get(iter->c_str(), &enabled) == S_OK)
                  {
                     techlog.EnableChannel(iter->c_str(), enabled != 0);
                  }
               }
            }
         }
      }
   }

   return result;
}

static tResult InitGlobalConfig(int argc, tChar * argv[])
{
   Assert(argc > 0);

   LoadCfgFile(argv[0], g_pConfig);
   LoadIniFile(argv[0], g_pConfig);

   ParseCommandLine(argc, argv, g_pConfig);

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

static tResult MainFrame()
{
   UseGlobal(Scheduler);
   pScheduler->NextFrame();

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

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

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      DebugMsg("One or more application-level services failed to start!\n");
      return false;
   }

   cAutoIPtr<ITask> pMainRenderTask(MainRenderTaskCreate());
   if (!!pMainRenderTask)
   {
      UseGlobal(Scheduler);
      pScheduler->AddRenderTask(pMainRenderTask);
   }

   cAutoIPtr<ITask> pMainInitTask(MainInitTaskCreate());
   if (!!pMainInitTask)
   {
      UseGlobal(Scheduler);
      pScheduler->AddFrameTask(pMainInitTask, 0, 1, 0);
   }

   TextFormatRegister(_T("css,lua,xml"));
   EngineRegisterResourceFormats();
   EngineRegisterScriptFunctions();
   TerrainRegisterResourceFormats();
   ImageRegisterResourceFormats();

   if (ConfigGet(_T("data"), &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddDirectoryTreeFlattened(temp.c_str());
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

   SysSetResizeCallback(ResizeHack);
   SysSetFrameCallback(MainFrame);

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
   cAutoIPtr<IGUIRenderDeviceContext> pGuiRenderDevice;
   if (GUIRenderDeviceCreateGL(&pGuiRenderDevice) == S_OK)
   {
      pGUIContext->SetRenderDeviceContext(pGuiRenderDevice);
   }

   SysAppActivate(true);

   UseGlobal(Camera);
   pCamera->SetPerspective(g_fov, (float)width / height, kZNear, kZFar);

   UseGlobal(Scheduler);
   pScheduler->Start();

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static void MainTerm()
{
   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   SafeRelease(g_pFrameStats);

   // This will make sure the GL context is destroyed
   SysQuit();

   StopGlobalObjects();
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

   int result = SysEventLoop(NULL);

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

   int result = SysEventLoop(NULL);

   MainTerm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////
