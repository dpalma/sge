///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORAPI_H
#define INCLUDED_EDITORAPI_H

#include "comtools.h"
#include "connpt.h"
#include "techstring.h"
#include "vec3.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(IEditorSplashScreen);
F_DECLARE_INTERFACE(IEditorApp);
F_DECLARE_INTERFACE(IEditorAppListener);
F_DECLARE_INTERFACE(IEditorToolState);
F_DECLARE_INTERFACE(IEditorToolStateListener);
F_DECLARE_INTERFACE(IEditorView);
F_DECLARE_INTERFACE(IEditorModel);
F_DECLARE_INTERFACE(IEditorModelListener);
F_DECLARE_INTERFACE(IEditorCommand);
F_DECLARE_INTERFACE(IEditorCompositeCommand);
F_DECLARE_INTERFACE(IEditorTool);

F_DECLARE_INTERFACE(ITerrainModel);
F_DECLARE_INTERFACE(ITerrainTileSet);

class cEditorKeyEvent;
class cEditorMouseEvent;
class cEditorMouseWheelEvent;

F_DECLARE_HANDLE(HTERRAINQUAD);
F_DECLARE_HANDLE(HTERRAINVERTEX);


#define UUID(uuidstr) __declspec(uuid(uuidstr))


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorSplashScreen
//

interface UUID("5193B882-4229-4396-A062-17029546F4C8") IEditorSplashScreen : IUnknown
{
};

////////////////////////////////////////

tResult EditorSplashScreenCreate(IEditorSplashScreen * * ppEditorSplashScreen);


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorApp
//

interface UUID("2A04E541-6BA1-41e9-92FA-E7B3D493F1A2") IEditorApp : IUnknown
{
   virtual tResult AddEditorAppListener(IEditorAppListener * pListener) = 0;
   virtual tResult RemoveEditorAppListener(IEditorAppListener * pListener) = 0;

   virtual tResult GetActiveModel(IEditorModel * * ppModel) = 0;

   virtual tResult SetDefaultTileSet(const tChar * pszTileSet) = 0;
   virtual tResult GetDefaultTileSet(cStr * pTileSet) const = 0;
};

////////////////////////////////////////

void EditorAppCreate();


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorAppListener
//

interface UUID("14E9EE21-4E6F-4b04-8F5C-742DFFA955BE") IEditorAppListener : IUnknown
{
   virtual tResult OnDefaultTileSetChange(const tChar * pszTileSet) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorToolState
//

interface UUID("8BF1683A-F953-47eb-A740-FCD4CBF07CC2") IEditorToolState : IUnknown
{
   virtual tResult AddToolStateListener(IEditorToolStateListener * pListener) = 0;
   virtual tResult RemoveToolStateListener(IEditorToolStateListener * pListener) = 0;

   virtual tResult HandleMessage(MSG * pMsg) = 0;

   virtual tResult GetActiveTool(IEditorTool * * ppTool) = 0;
   virtual tResult SetActiveTool(IEditorTool * pTool) = 0;

   virtual tResult GetToolCapture(IEditorTool * * ppTool) = 0;
   virtual tResult SetToolCapture(IEditorTool * pTool) = 0;
   virtual tResult ReleaseToolCapture() = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorToolStateListener
//

interface UUID("98774B96-677A-48d4-8BCC-3927D4CCC8D4") IEditorToolStateListener : IUnknown
{
   virtual tResult OnActiveToolChange(IEditorTool * pNewTool, IEditorTool * pFormerTool) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorView
//

interface UUID("78C29790-865D-4f81-9AF1-26EC23BB5FAC") IEditorView : IUnknown
{
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorModel
//

interface UUID("F131D72E-30A7-4758-A094-830F00A50D91") IEditorModel : IUnknown
{
   virtual tResult AddCommand(IEditorCommand * pCommand, bool bDo) = 0;

   virtual tResult AddEditorModelListener(IEditorModelListener * pListener) = 0;
   virtual tResult RemoveEditorModelListener(IEditorModelListener * pListener) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorModelListener
//

interface UUID("A0F96E27-9D96-424b-96AB-139704087F13") IEditorModelListener : IUnknown
{
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorCommand
//

interface UUID("936BD53E-35B5-4f72-AFA4-AE304122E7D4") IEditorCommand : IUnknown
{
   virtual tResult Do() = 0;

   virtual tResult CanUndo() = 0;

   virtual tResult Undo() = 0;

   virtual tResult GetLabel(cStr * pLabel) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorCompositeCommand
//

interface UUID("A0518634-6279-4aeb-879F-E2D560F58626") IEditorCompositeCommand : IEditorCommand
{
   virtual tResult Add(IEditorCommand * pCommand) = 0;
   virtual tResult Remove(IEditorCommand * pCommand) = 0;
};

////////////////////////////////////////

tResult EditorCompositeCommandCreate(IEditorCompositeCommand * * ppCommand);

////////////////////////////////////////

enum eEditorCompositeCommandCallback
{
   kPreDo,
   kPostDo,
   kPreUndo,
   kPostUndo
};

typedef void (* tEditorCompositeCommandCallback)(eEditorCompositeCommandCallback type);

tResult EditorCompositeCommandCreate(tEditorCompositeCommandCallback pfnCallback,
                                     IEditorCompositeCommand * * ppCommand);


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorTool
//

////////////////////////////////////////

enum eEditorToolResult
{
   S_EDITOR_TOOL_HANDLED   = S_OK,
   S_EDITOR_TOOL_CONTINUE  = S_FALSE,
};

////////////////////////////////////////

interface UUID("AFAD398E-14D6-4eed-B503-AFE44C6989C0") IEditorTool : IUnknown
{
   virtual tResult Activate() = 0;
   virtual tResult Deactivate() = 0;

   virtual tResult OnKeyDown(const cEditorKeyEvent & keyEvent) = 0;
   virtual tResult OnKeyUp(const cEditorKeyEvent & keyEvent) = 0;
   virtual tResult OnLButtonDblClk(const cEditorMouseEvent & mouseEvent) = 0;
   virtual tResult OnLButtonDown(const cEditorMouseEvent & mouseEvent) = 0;
	virtual tResult OnLButtonUp(const cEditorMouseEvent & mouseEvent) = 0;
   virtual tResult OnRButtonDblClk(const cEditorMouseEvent & mouseEvent) = 0;
	virtual tResult OnRButtonDown(const cEditorMouseEvent & mouseEvent) = 0;
	virtual tResult OnRButtonUp(const cEditorMouseEvent & mouseEvent) = 0;
	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent) = 0;
   virtual tResult OnMouseWheel(const cEditorMouseWheelEvent & mouseWheelEvent) = 0;

   virtual tResult GetToolTip(const cEditorMouseEvent & mouseEvent, cStr * pToolTipText, uint_ptr * pToolTipId) const = 0;
};

////////////////////////////////////////

class cDefaultEditorTool : public IEditorTool
{
public:
   virtual tResult Activate() { return S_OK; }
   virtual tResult Deactivate() { return S_OK; }

   virtual tResult OnKeyDown(const cEditorKeyEvent & keyEvent) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnKeyUp(const cEditorKeyEvent & keyEvent) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnLButtonDblClk(const cEditorMouseEvent & mouseEvent) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnLButtonDown(const cEditorMouseEvent & mouseEvent) { return S_EDITOR_TOOL_CONTINUE; }
	virtual tResult OnLButtonUp(const cEditorMouseEvent & mouseEvent) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnRButtonDblClk(const cEditorMouseEvent & mouseEvent) { return S_EDITOR_TOOL_CONTINUE; }
	virtual tResult OnRButtonDown(const cEditorMouseEvent & mouseEvent) { return S_EDITOR_TOOL_CONTINUE; }
	virtual tResult OnRButtonUp(const cEditorMouseEvent & mouseEvent) { return S_EDITOR_TOOL_CONTINUE; }
	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnMouseWheel(const cEditorMouseWheelEvent & mouseWheelEvent) { return S_EDITOR_TOOL_CONTINUE; }

   virtual tResult GetToolTip(const cEditorMouseEvent & mouseEvent, cStr * pToolTipText, uint_ptr * pToolTipId) const { return S_FALSE; }
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORAPI_H
