/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorApp.h"
#include "editorDoc.h"
#include "editorView.h"
#include "editorTypes.h"

#include "scriptapi.h"

#include "resourceapi.h"
#include "resmgr.h"
#include "configapi.h"
#include "techtime.h"
#include "connptimpl.h"
#include "readwriteapi.h"
#include "str.h"

#include <algorithm>
#include <zmouse.h>

#ifdef HAVE_CPPUNIT
#ifdef USE_MFC_TESTRUNNER
#include <cppunit/ui/mfc/TestRunner.h>
#else
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestFailure.h>
#include <cppunit/SourceLine.h>
#include <cppunit/Exception.h>
#endif
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#endif

#include "resource.h"       // main symbols

#include "dbgalloc.h" // must be last header

extern sScriptReg g_editorCmds[];
extern uint g_nEditorCmds;

static const tChar g_szRegistryKey[] = _T("SGE");

/////////////////////////////////////////////////////////////////////////////

static const SIZE g_mapSizes[] =
{
   { 64, 64 },
   { 128, 128 },
   { 192, 192 },
   { 256, 256 },
};

static const uint kDefaultMapSizeIndex = 0;

/////////////////////////////////////////////////////////////////////////////

template <typename CONTAINER>
void ListTileSets(CONTAINER * pContainer)
{
   UseGlobal(EditorTileManager);

   uint nTileSets = 0;
   if (pEditorTileManager->GetTileSetCount(&nTileSets) == S_OK && nTileSets > 0)
   {
      for (uint i = 0; i < nTileSets; i++)
      {
         cAutoIPtr<IEditorTileSet> pTileSet;
         if (pEditorTileManager->GetTileSet(i, &pTileSet) == S_OK)
         {
            Assert(!!pTileSet);
            cStr name;
            Verify(pTileSet->GetName(&name) == S_OK);
            pContainer->push_back(name);
         }
         else
         {
            WarnMsg1("Error getting tile set %d\n", i);
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////

static char * GetEntireContents(IReader * pReader)
{
   Assert(pReader != NULL);

   pReader->Seek(0, kSO_End);
   ulong length;
   pReader->Tell(&length);
   pReader->Seek(0, kSO_Set);

   char * pszContents = new char[length + 1];

   if (pReader->Read(pszContents, length) != S_OK)
   {
      delete [] pszContents;
      return NULL;
   }

   pszContents[length] = 0;

   return pszContents;
}

////////////////////////////////////////

static bool ScriptExecResource(IScriptInterpreter * pInterpreter, const char * pszResource)
{
   bool bResult = false;

   cAutoIPtr<IResource> pResource;
   UseGlobal(ResourceManager2);
   if (pResourceManager2->Load(tResKey(pszResource, kRC_Text), &pResource) == S_OK)
   {
      char * pszCode;
      if (pResource->GetData((void**)&pszCode) == S_OK)
      {
         bResult = SUCCEEDED(pInterpreter->ExecString(pszCode));
         delete [] pszCode;
      }
   }

   return bResult;
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorApp
//

////////////////////////////////////////

BEGIN_CONSTRAINTS()
   AFTER_GUID(IID_IResourceManager)
   AFTER_GUID(IID_IResourceManager2)
   AFTER_GUID(IID_IScriptInterpreter)
END_CONSTRAINTS()

////////////////////////////////////////

cEditorApp::cEditorApp()
 : cGlobalObject<IMPLEMENTS(IEditorApp)>("EditorApp", CONSTRAINTS()),
   m_hCurrentToolWnd(NULL)
{
}

////////////////////////////////////////

cEditorApp::~cEditorApp()
{
}

////////////////////////////////////////

tResult cEditorApp::Init()
{
   if (!m_mainWnd.CreateEx())
   {
      return E_FAIL;
   }

   ScriptAddFunctions(g_editorCmds, g_nEditorCmds);

   cStr temp;
   if (ConfigGet("data", &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddSearchPath(temp);
   }

   cStr autoexecScript("editor.lua");
   ConfigGet("editor_autoexec_script", &autoexecScript);

   UseGlobal(ScriptInterpreter);
   if (pScriptInterpreter->ExecFile(autoexecScript) != S_OK)
   {
      if (!ScriptExecResource(pScriptInterpreter, autoexecScript))
      {
         WarnMsg1("Error parsing or executing %s\n", autoexecScript.c_str());
      }
   }

   if (pScriptInterpreter->CallFunction("EditorInit") != S_OK)
   {
      ErrorMsg("An error occured calling EditorInit()\n");
   }

   CMessageLoop * pMessageLoop = _Module.GetMessageLoop();
   pMessageLoop->AddMessageFilter(this);

	m_mainWnd.ShowWindow(SW_SHOW);
	m_mainWnd.UpdateWindow();

   return S_OK;
}

////////////////////////////////////////

tResult cEditorApp::Term()
{
	if (m_mainWnd.IsWindow())
   {
      m_mainWnd.DestroyWindow();
   }

   CMessageLoop * pMessageLoop = _Module.GetMessageLoop();
   pMessageLoop->RemoveMessageFilter(this);

   return S_OK;
}

////////////////////////////////////////

void EditorAppCreate()
{
   cAutoIPtr<IEditorApp>(new cEditorApp);
}

////////////////////////////////////////

BOOL cEditorApp::PreTranslateMessage(MSG * pMsg) 
{
   cAutoIPtr<IEditorTool> pTool;
   if (AccessToolCapture() != NULL)
   {
      pTool = CTAddRef(AccessToolCapture());
   }
   else if (AccessActiveTool() != NULL)
   {
      pTool = CTAddRef(AccessActiveTool());
   }
//   else if (AccessDefaultTool() != NULL)
//   {
//      pTool = CTAddRef(AccessDefaultTool());
//   }

   if (!!pTool)
   {
      m_hCurrentToolWnd = pMsg->hwnd;

#if 0 // TODO
      Assert(!m_pCurrentToolView);
      CWnd * pWnd = CWnd::FromHandlePermanent(pMsg->hwnd);
      if (pWnd != NULL)
      {
         cEditorView * pEditorView = DYNAMIC_DOWNCAST(cEditorView, pWnd);
         if (pEditorView != NULL)
         {
            m_pCurrentToolView = CTAddRef(static_cast<IEditorView *>(pEditorView));
         }
      }
#else
      cAutoIPtr<IEditorView> pView;
      if (SendMessage(pMsg->hwnd, WM_GET_IEDITORVIEW, 0, (LPARAM)&pView) == 0)
      {
         m_pCurrentToolView = pView;
      }
#endif

      tResult toolResult = S_EDITOR_TOOL_CONTINUE;

      switch (pMsg->message)
      {
         case WM_KEYDOWN:
         {
            toolResult = pTool->OnKeyDown(cEditorKeyEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);

            if ((toolResult == S_EDITOR_TOOL_CONTINUE) && (pMsg->wParam == VK_ESCAPE))
            {
               SetActiveTool(NULL);
            }
            break;
         }

         case WM_KEYUP:
         {
            toolResult = pTool->OnKeyUp(cEditorKeyEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_LBUTTONDBLCLK:
         {
            toolResult = pTool->OnLButtonDblClk(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_LBUTTONDOWN:
         {
            toolResult = pTool->OnLButtonDown(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_LBUTTONUP:
         {
            toolResult = pTool->OnLButtonUp(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_RBUTTONDBLCLK:
         {
            toolResult = pTool->OnRButtonDblClk(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_RBUTTONDOWN:
         {
            toolResult = pTool->OnRButtonDown(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_RBUTTONUP:
         {
            toolResult = pTool->OnRButtonUp(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_MOUSEMOVE:
         {
            toolResult = pTool->OnMouseMove(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_MOUSEWHEEL:
         {
            toolResult = pTool->OnMouseWheel(cEditorMouseWheelEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }
      }

      m_hCurrentToolWnd = NULL;
      SafeRelease(m_pCurrentToolView);

      if (toolResult == S_EDITOR_TOOL_HANDLED)
      {
         return TRUE;
      }
   }

   return FALSE;
}

////////////////////////////////////////

tResult cEditorApp::AddLoopClient(IEditorLoopClient * pLoopClient)
{
   return add_interface(m_loopClients, pLoopClient) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::RemoveLoopClient(IEditorLoopClient * pLoopClient)
{
   return remove_interface(m_loopClients, pLoopClient) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::CallLoopClients(double time, double elapsed)
{
   tEditorLoopClients::iterator iter = m_loopClients.begin();
   tEditorLoopClients::iterator end = m_loopClients.end();
   for (; iter != end; iter++)
   {
      (*iter)->OnFrame(time, elapsed);
   }
   return S_OK;
}

////////////////////////////////////////

tResult cEditorApp::AddEditorAppListener(IEditorAppListener * pListener)
{
   return add_interface(m_editorAppListeners, pListener) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::RemoveEditorAppListener(IEditorAppListener * pListener)
{
   return remove_interface(m_editorAppListeners, pListener) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::GetActiveTool(IEditorTool * * ppTool)
{
   return m_pActiveTool.GetPointer(ppTool);
}

////////////////////////////////////////

tResult cEditorApp::SetActiveTool(IEditorTool * pTool)
{
   tEditorAppListeners::iterator iter;
   for (iter = m_editorAppListeners.begin(); iter != m_editorAppListeners.end(); iter++)
   {
      (*iter)->OnActiveToolChange(pTool, m_pActiveTool);
   }

   if (!!m_pActiveTool)
   {
      m_pActiveTool->Deactivate();
   }

   SafeRelease(m_pActiveTool);

   if (pTool != NULL)
   {
      pTool->Activate();
      m_pActiveTool = CTAddRef(pTool);
   }

   return S_OK;
}

////////////////////////////////////////

tResult cEditorApp::GetDefaultTool(IEditorTool * * ppTool)
{
   return m_pDefaultTool.GetPointer(ppTool);
}

////////////////////////////////////////

tResult cEditorApp::SetDefaultTool(IEditorTool * pTool)
{
   if (pTool == NULL)
   {
      return E_POINTER;
   }

   SafeRelease(m_pDefaultTool);

   m_pDefaultTool = CTAddRef(pTool);

   return S_OK;
}

////////////////////////////////////////

tResult cEditorApp::GetToolCapture(IEditorTool * * ppTool)
{
   return m_pToolCapture.GetPointer(ppTool);
}

////////////////////////////////////////

tResult cEditorApp::SetToolCapture(IEditorTool * pTool)
{
   if (pTool == NULL)
   {
      return E_POINTER;
   }

   if (!IsWindow(m_hCurrentToolWnd))
   {
      DebugMsg("No valid window handle in SetToolCapture\n");
      return E_FAIL;
   }

   ::SetCapture(m_hCurrentToolWnd);

   SafeRelease(m_pToolCapture);

   m_pToolCapture = CTAddRef(pTool);

   return S_OK;
}

////////////////////////////////////////

tResult cEditorApp::ReleaseToolCapture()
{
   if (!m_pToolCapture)
   {
      return S_FALSE;
   }
   else
   {
      ReleaseCapture();
      SafeRelease(m_pToolCapture);
      return S_OK;
   }
}

////////////////////////////////////////

void RunUnitTests()
{
#ifdef HAVE_CPPUNIT
#ifdef USE_MFC_TESTRUNNER
   CppUnit::MfcUi::TestRunner runner;
   runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
   runner.run();
#else
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
   }
   else
   {
      techlog.Print(kInfo, "%d unit tests succeeded\n", runner.result().tests().size());
   }
#endif
#else
   AtlMessageBox(GetFocus(), IDS_NO_UNIT_TESTS, IDR_MAINFRAME);
#endif
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cEditorAppTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cEditorAppTests);
   CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cEditorAppTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
