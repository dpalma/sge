/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORTOOLSTATE_H)
#define INCLUDED_EDITORTOOLSTATE_H

#include "editorapi.h"

#include "connptimpl.h"
#include "globalobjdef.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorToolState
//

class cEditorToolState : public cComObject2<IMPLEMENTSCP(IEditorToolState, IEditorToolStateListener),
                                            IMPLEMENTS(IGlobalObject)>
{
public:
	cEditorToolState();
	~cEditorToolState();

   DECLARE_NAME(EditorToolState)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult AddToolStateListener(IEditorToolStateListener * pListener)
   {
      return Connect(pListener);
   }

   virtual tResult RemoveToolStateListener(IEditorToolStateListener * pListener)
   {
      return Disconnect(pListener);
   }

	virtual tResult HandleMessage(MSG * pMsg);

   virtual tResult GetActiveTool(IEditorTool * * ppTool);
   virtual tResult SetActiveTool(IEditorTool * pTool);

   virtual tResult GetToolCapture(IEditorTool * * ppTool);
   virtual tResult SetToolCapture(IEditorTool * pTool);
   virtual tResult ReleaseToolCapture();

private:
   IEditorTool * AccessActiveTool();
   IEditorTool * AccessToolCapture();

   HWND m_hCurrentToolWnd; // the HWND of the view above

   cAutoIPtr<IEditorTool> m_pActiveTool;
   cAutoIPtr<IEditorTool> m_pToolCapture;
};

////////////////////////////////////////

IEditorTool * cEditorToolState::AccessActiveTool()
{
   return static_cast<IEditorTool *>(m_pActiveTool);
}

////////////////////////////////////////

IEditorTool * cEditorToolState::AccessToolCapture()
{
   return static_cast<IEditorTool *>(m_pToolCapture);
}


/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTOOLSTATE_H)
