///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ai/aiagentapi.h"
#include "engine/cameraapi.h"
#include "engine/engineapi.h"
#include "engine/entityapi.h"
#include "engine/saveloadapi.h"
#include "engine/scenarioapi.h"
#include "engine/terrainapi.h"
#include "gui/guiapi.h"
#include "gui/guielementapi.h"
#include "network/netapi.h"
#include "platform/inputapi.h"
#include "platform/keys.h"
#include "platform/sys.h"
#include "render/renderapi.h"
#include "script/scriptapi.h"
#include "sound/soundapi.h"

#include "tech/resourceapi.h"
#include "tech/configapi.h"
#include "tech/filespec.h"
#include "tech/filepath.h"
#include "tech/techstring.h"
#include "tech/globalobj.h"
#include "tech/multivar.h"
#include "tech/ray.h"
#include "tech/schedulerapi.h"
#include "tech/simapi.h"
#include "tech/statemachine.h"
#include "tech/statemachinetem.h"
#include "tech/techtime.h"
#include "tech/thread.h"
#include "tech/threadcallapi.h"
#include "tech/imageapi.h"

#include <ctime>

#include "tech/dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' used in base member initializer list

// The following definitions are required for WinMain
F_DECLARE_HANDLE(HINSTANCE);
typedef char * LPSTR;


///////////////////////////////////////////////////////////////////////////////

#define kAutoExecScript _T("autoexec.lua")
#define kDefaultWidth   800
#define kDefaultHeight  600
#define kDefaultBpp     16


///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<IGUILabelElement> g_pFrameStats;


///////////////////////////////////////////////////////////////////////////////

#define kRT_Dictionary _T("Dictionary")

void * DictionaryLoad(IReader * pReader, void * typeParam)
{
   if (pReader == NULL || typeParam == NULL)
   {
      return NULL;
   }

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(pReader);
   if (!pStore)
   {
      return NULL;
   }

   IDictionary * pDict = (IDictionary *)typeParam;

   if (pStore->Load(pDict) == S_OK)
   {
      return pDict;
   }
   else
   {
      return NULL;
   }
}

void DictionaryUnload(void * pData)
{
   // Nothing to unload
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMainInputListener
//

class cMainInputListener : public cComObject<IMPLEMENTS(IInputListener)>
{
public:
   virtual bool OnInputEvent(const sInputEvent * pEvent);
};

///////////////////////////////////////

bool cMainInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   UseGlobal(GUIContext);
   if (!!pGUIContext && pGUIContext->HandleInputEvent(pEvent))
   {
      return true;
   }

   UseGlobal(CameraControl);
   if (!!pCameraControl && pCameraControl->HandleInputEvent(pEvent))
   {
      return true;
   }

   if (pEvent->down && pEvent->key == kMouseLeft)
   {
      UseGlobal(Renderer);

      cAutoIPtr<IRenderCamera> pCamera;
      cRay pickRay;
      if (pRenderer->GetCamera(&pCamera) == S_OK
         && pCamera->GenerateScreenPickRay(pEvent->point.x, pEvent->point.y, &pickRay) == S_OK)
      {
         cAutoIPtr<IEntity> pEntity;
         UseGlobal(EntityManager);
         if (pEntityManager->RayCast(pickRay, &pEntity) == S_OK)
         {
            pEntityManager->Select(pEntity);
         }
         else
         {
            pEntityManager->DeselectAll();
         }
      }
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMainRenderTask
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

   UseGlobal(TerrainRenderer);
   pTerrainRenderer->Render();

   UseGlobal(EntityManager);
   pEntityManager->RenderAll();

   int width, height;
   if (SysGetWindowSize(&width, &height) == S_OK)
   {
      if (pRenderer->Begin2D(width, height) == S_OK)
      {
         UseGlobal(GUIContext);
         pGUIContext->RenderGUI(width, height);

         pRenderer->End2D();
      }
   }

   pRenderer->EndScene();

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
// CLASS: cUnitTestThread
//

class cUnitTestThread : public cThread
{
public:
   cUnitTestThread(bool bAutoDeleteSelf = false);
   ~cUnitTestThread();

   virtual int Run();

private:
   bool m_bAutoDeleteSelf;
   tThreadId m_outerThreadId;
};

////////////////////////////////////////

cUnitTestThread::cUnitTestThread(bool bAutoDeleteSelf)
 : m_bAutoDeleteSelf(bAutoDeleteSelf)
 , m_outerThreadId(ThreadGetCurrentId())
{
}

////////////////////////////////////////

cUnitTestThread::~cUnitTestThread()
{
}

////////////////////////////////////////

int cUnitTestThread::Run()
{
   ThreadSetName(GetThreadId(), "UnitTestRunnerThread");

   tResult result = SysRunUnitTests();

   if (FAILED(result))
   {
      UseGlobal(ThreadCaller);
      pThreadCaller->PostCall(m_outerThreadId, &SysQuit);
   }

   if (m_bAutoDeleteSelf)
   {
      delete this;
   }

   return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMainInitTask
//

class cMainInitTask : public cComObject<IMPLEMENTS(ITask)>
                    , public cStateMachine<cMainInitTask, double>
{
public:
   cMainInitTask(const tChar * pszArgv0);
   ~cMainInitTask();

   virtual tResult Execute(double time);

   void OnInitialStateUpdate(double);
   void OnEnterErrorState();
   void OnRunInitStages(double);
   void OnEnterFinishedState();

private:
   tResult SetupResourceManager();
   tResult RunStartupScript();
   tResult CreateMainWindow();

   cStr m_argv0;

   cUnitTestThread * m_pUnitTestThread;

   typedef tResult (cMainInitTask::*tInitStageFn)();

   tInitStageFn m_stages[4];
   int m_currentStage;

   tState m_initialState;
   tState m_errorState;
   tState m_stagedInitState;
   tState m_finishedState;
};

////////////////////////////////////////

cMainInitTask::cMainInitTask(const tChar * pszArgv0)
 : tStateMachine(&m_initialState)
 , m_argv0((pszArgv0 != NULL) ? pszArgv0 : _T(""))
 , m_pUnitTestThread(NULL)
 , m_currentStage(0)
 , m_initialState(NULL, NULL, &cMainInitTask::OnInitialStateUpdate)
 , m_errorState(&cMainInitTask::OnEnterErrorState, NULL, NULL)
 , m_stagedInitState(NULL, NULL, &cMainInitTask::OnRunInitStages)
 , m_finishedState(&cMainInitTask::OnEnterFinishedState, NULL, NULL)
{
   memset(m_stages, 0, sizeof(m_stages));
   m_stages[0] = &cMainInitTask::SetupResourceManager;
   m_stages[1] = &cMainInitTask::RunStartupScript;
   m_stages[2] = &cMainInitTask::CreateMainWindow;
}

////////////////////////////////////////

cMainInitTask::~cMainInitTask()
{
   delete m_pUnitTestThread, m_pUnitTestThread = NULL;
}

////////////////////////////////////////

tResult cMainInitTask::Execute(double time)
{
   Update(time);

   return IsCurrentState(&m_finishedState) ? S_FALSE : S_OK;
}

////////////////////////////////////////

void cMainInitTask::OnInitialStateUpdate(double)
{
   Assert(m_pUnitTestThread == NULL);
   m_pUnitTestThread = new cUnitTestThread(false);
   if (m_pUnitTestThread != NULL)
   {
      if (!m_pUnitTestThread->Create())
      {
         delete m_pUnitTestThread;
      }
   }

   GotoState(&m_stagedInitState);
}

////////////////////////////////////////

void cMainInitTask::OnEnterErrorState()
{
   SysQuit();
   GotoState(&m_finishedState);
}

////////////////////////////////////////

void cMainInitTask::OnRunInitStages(double)
{
   Assert(m_currentStage >= 0);

   tInitStageFn pfnStage = m_stages[m_currentStage];
   if (pfnStage != NULL)
   {
      if (FAILED((this->*pfnStage)()))
      {
         GotoState(&m_errorState);
         return;
      }
   }

   if (++m_currentStage >= _countof(m_stages))
   {
      GotoState(&m_finishedState);
   }
}

////////////////////////////////////////

void cMainInitTask::OnEnterFinishedState()
{
   if (m_pUnitTestThread != NULL)
   {
      m_pUnitTestThread->Join();
   }
}

////////////////////////////////////////

tResult cMainInitTask::SetupResourceManager()
{
   TextFormatRegister(_T("cfg,css,lua,xml"));
   EngineRegisterResourceFormats();
   TerrainRegisterResourceFormats();
   ImageRegisterResourceFormats();

   UseGlobal(ResourceManager);
   pResourceManager->RegisterFormat(kRT_Dictionary, _T("cfg"),
      DictionaryLoad, NULL, DictionaryUnload, g_pConfig);

   cStr temp;
   if (ConfigGet(_T("data"), &temp) == S_OK)
   {
      if (pResourceManager->AddArchive(temp.c_str()) != S_OK)
      {
         pResourceManager->AddDirectoryTreeFlattened(temp.c_str());
      }
   }

   cFileSpec config(m_argv0.c_str());
   config.SetFileExt(_T("cfg"));
   void * pUnused = NULL;
   pResourceManager->Load(config.GetFileName(), kRT_Dictionary, NULL, &pUnused);

   return S_OK;
}

////////////////////////////////////////

tResult cMainInitTask::RunStartupScript()
{
   EngineRegisterScriptFunctions();

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
            return E_FAIL;
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

   return S_OK;
}

////////////////////////////////////////

tResult cMainInitTask::CreateMainWindow()
{
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

   if (SysCreateWindow(_T("Game"), width, height) != S_OK)
   {
      return E_FAIL;
   }

   UseGlobal(Renderer);
   cAutoIPtr<IRenderTarget> pRenderTarget;

#ifdef _WIN32
   if (pRenderer->CreateRenderTarget(SysGetMainWindow(), &pRenderTarget) != S_OK)
#else
   if (pRenderer->CreateRenderTarget(SysGetDisplay(), SysGetMainWindow(), &pRenderTarget) != S_OK)
#endif
   {
      return E_FAIL;
   }

   if (pRenderer->SetRenderTarget(pRenderTarget) != S_OK)
   {
      return E_FAIL;
   }

   cAutoIPtr<ITask> pMainRenderTask(MainRenderTaskCreate());
   if (!!pMainRenderTask)
   {
      UseGlobal(Scheduler);
      pScheduler->AddRenderTask(pMainRenderTask);
   }

   cAutoIPtr<IInputListener> pInputListener(new cMainInputListener);
   if (!!pInputListener)
   {
      UseGlobal(Input);
      pInput->AddInputListener(pInputListener);
   }

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   AIAgentMessageRouterCreate();
   CameraControlCreate();
   EntityCommandManagerCreate();
   EntityCommandUICreate();
   EntityComponentRegistryCreate();
   EntityFactoryCreate();
   EntityManagerCreate();
   InputCreate();
   GUIContextCreate();
   GUIFactoryCreate();
   GUIEventSoundsCreate();
   NetworkCreate();
   RendererCreate();
   ResourceManagerCreate();
   SaveLoadManagerCreate();
   ScenarioCreate();
   SchedulerCreate();
   ScriptInterpreterCreate();
   SimCreate();
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

   // If no data directory was given, try to look for the default (sort of a hack)
   cStr temp;
   if (ConfigGet(_T("data"), &temp) != S_OK)
   {
      cFilePath path;
      if (cFileSpec(argv[0]).GetPath(&path))
      {
         for (int i = 0; i < 3; ++i)
         {
            // On the first try (i == 0), add just "data" with no parent directory ("..")
            if (i > 0)
            {
               path.AddRelative(_T(".."));
            }
            cFilePath tempPath(path);
            tempPath.AddRelative("data");
            tempPath = tempPath.CollapseDots();
            if (FilePathExists(tempPath.CollapseDots()))
            {
               WarnMsg1("No data directory specified; Using \"%s\"\n", tempPath.CStr());
               g_pConfig->Set(_T("data"), tempPath.CStr());
               break;
            }
         }
      }
   }

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

   cAutoIPtr<ITask> pMainInitTask(static_cast<ITask *>(new cMainInitTask(argv[0])));
   if (!!pMainInitTask)
   {
      UseGlobal(Scheduler);
      pScheduler->AddFrameTask(pMainInitTask, 0, 1, 0);
   }

   UseGlobal(ThreadCaller);
   if (FAILED(pThreadCaller->ThreadInit()))
   {
      return false;
   }

   UseGlobal(Scheduler);
   pScheduler->Start();

   SysAppActivate(true);

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

   int result = SysEventLoop(NULL, kSELF_RunScheduler | kSELF_ReceiveThreadCalls);

   MainTerm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(__GNUC__)
int main(int argc, char * argv[])
{
   if (!MainInit(argc, argv))
   {
      MainTerm();
      return EXIT_FAILURE;
   }

   int result = SysEventLoop(NULL, kSELF_RunScheduler | kSELF_ReceiveThreadCalls);

   MainTerm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////
