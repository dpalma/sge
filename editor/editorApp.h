/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORAPP_H)
#define INCLUDED_EDITORAPP_H

#include "editorapi.h"
#include "MainFrm.h"

#include "globalobj.h"

#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorApp
//

class cEditorApp : public cGlobalObject<IMPLEMENTS(IEditorApp)>,
                   public CMessageFilter
{
public:
	cEditorApp();
   ~cEditorApp();

   virtual tResult Init();
   virtual tResult Term();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

   virtual tResult AddLoopClient(IEditorLoopClient * pLoopClient);
   virtual tResult RemoveLoopClient(IEditorLoopClient * pLoopClient);

   virtual tResult CallLoopClients(double time, double elapsed);

   virtual tResult AddEditorAppListener(IEditorAppListener * pListener);
   virtual tResult RemoveEditorAppListener(IEditorAppListener * pListener);

   virtual tResult GetActiveTool(IEditorTool * * ppTool);
   virtual tResult SetActiveTool(IEditorTool * pTool);

   virtual tResult GetDefaultTool(IEditorTool * * ppTool);
   virtual tResult SetDefaultTool(IEditorTool * pTool);

   virtual tResult GetToolCapture(IEditorTool * * ppTool);
   virtual tResult SetToolCapture(IEditorTool * pTool);
   virtual tResult ReleaseToolCapture();

private:
   IEditorTool * AccessActiveTool();
   IEditorTool * AccessDefaultTool();
   IEditorTool * AccessToolCapture();

   typedef std::vector<IEditorLoopClient *> tEditorLoopClients;
   tEditorLoopClients m_loopClients;

   typedef std::vector<IEditorAppListener *> tEditorAppListeners;
   tEditorAppListeners m_editorAppListeners;

   cAutoIPtr<IEditorView> m_pCurrentToolView; // the view passed to the tool being processed
   HWND m_hCurrentToolWnd; // the HWND of the view above

   cAutoIPtr<IEditorTool> m_pActiveTool;
   cAutoIPtr<IEditorTool> m_pDefaultTool;
   cAutoIPtr<IEditorTool> m_pToolCapture;

   cMainFrame m_mainWnd;
};

////////////////////////////////////////

IEditorTool * cEditorApp::AccessActiveTool()
{
   return static_cast<IEditorTool *>(m_pActiveTool);
}

////////////////////////////////////////

IEditorTool * cEditorApp::AccessDefaultTool()
{
   return static_cast<IEditorTool *>(m_pDefaultTool);
}

////////////////////////////////////////

IEditorTool * cEditorApp::AccessToolCapture()
{
   return static_cast<IEditorTool *>(m_pToolCapture);
}

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORAPP_H)
