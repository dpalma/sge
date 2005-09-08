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
F_DECLARE_INTERFACE(IEditorLoopClient);
F_DECLARE_INTERFACE(IEditorAppListener);
F_DECLARE_INTERFACE(IEditorTileSets);
F_DECLARE_INTERFACE(IEditorTileSetsListener);
F_DECLARE_INTERFACE(IEditorView);
F_DECLARE_INTERFACE(IEditorModel);
F_DECLARE_INTERFACE(IEditorModelListener);
F_DECLARE_INTERFACE(IEditorCommand);
F_DECLARE_INTERFACE(IEditorTerrainTileCommand);
F_DECLARE_INTERFACE(IEditorTool);

F_DECLARE_INTERFACE(ITerrainModel);
F_DECLARE_INTERFACE(ITerrainTileSet);

class cEditorKeyEvent;
class cEditorMouseEvent;
class cEditorMouseWheelEvent;

class cRay;

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
   virtual tResult AddLoopClient(IEditorLoopClient * pLoopClient) = 0;
   virtual tResult RemoveLoopClient(IEditorLoopClient * pLoopClient) = 0;

   virtual tResult AddEditorAppListener(IEditorAppListener * pListener) = 0;
   virtual tResult RemoveEditorAppListener(IEditorAppListener * pListener) = 0;

   virtual tResult GetActiveView(IEditorView * * ppView) = 0;
   virtual tResult GetActiveModel(IEditorModel * * ppModel) = 0;

   virtual tResult GetActiveTool(IEditorTool * * ppTool) = 0;
   virtual tResult SetActiveTool(IEditorTool * pTool) = 0;

   virtual tResult GetToolCapture(IEditorTool * * ppTool) = 0;
   virtual tResult SetToolCapture(IEditorTool * pTool) = 0;
   virtual tResult ReleaseToolCapture() = 0;
};

////////////////////////////////////////

void EditorAppCreate();


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorLoopClient
//

interface UUID("ED1B3A1A-E2D8-4eec-AABD-648A548729E8") IEditorLoopClient : IUnknown
{
   virtual void OnFrame(double time, double elapsed) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorAppListener
//

interface UUID("14E9EE21-4E6F-4b04-8F5C-742DFFA955BE") IEditorAppListener : IUnknown
{
   virtual tResult OnActiveToolChange(IEditorTool * pNewTool, IEditorTool * pFormerTool) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorTileSets
//
// Holds the set of all available terrain tile sets for use by editor tools.
// The actual tile sets (ITerrainTileSet) are loaded using the resource manager.

interface UUID("C0CDE13A-010A-4dc6-A9C5-75DC3373288D") IEditorTileSets : IUnknown
{
   DECLARE_CONNECTION_POINT(IEditorTileSetsListener);

   virtual tResult AddTileSet(const tChar * pszTileSet) = 0;

   virtual tResult GetTileSetCount(uint * pTileSetCount) const = 0;
   virtual tResult GetTileSet(uint index, cStr * pTileSet) const = 0;

   virtual tResult SetDefaultTileSet(const tChar * pszTileSet) = 0;
   virtual tResult GetDefaultTileSet(cStr * pTileSet) const = 0;
};

////////////////////////////////////////

void EditorTileSetsCreate();


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorTileSetsListener
//

interface UUID("CA332F5D-1E25-4825-ABD8-59C753B68544") IEditorTileSetsListener : IUnknown
{
   virtual void OnSetDefaultTileSet(const tChar * pszTileSet) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorView
//

interface UUID("78C29790-865D-4f81-9AF1-26EC23BB5FAC") IEditorView : IUnknown
{
   virtual tVec3 GetCameraEyePosition() const = 0;
   virtual tResult GetCameraPlacement(float * px, float * pz) = 0;
   virtual tResult PlaceCamera(float x, float z) = 0;
   virtual tResult GetCameraElevation(float * pElevation) = 0;
   virtual tResult SetCameraElevation(float elevation) = 0;

   virtual tResult GeneratePickRay(float ndx, float ndy, cRay * pRay) = 0;

   virtual tResult GetModel(IEditorModel * * ppModel) = 0;

   virtual tResult GetHighlightTile(int * piTileX, int * piTileZ) const = 0;
   virtual tResult HighlightTile(int iTileX, int iTileZ) = 0;
   virtual tResult ClearTileHighlight() = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorModel
//

interface UUID("F131D72E-30A7-4758-A094-830F00A50D91") IEditorModel : IUnknown
{
   virtual tResult AddCommand(IEditorCommand * pCommand) = 0;

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

   /// @brief Answers the question "can this command be undone at the same time as the given one?"
   virtual tResult Compare(IEditorCommand * pOther) = 0;
};


/////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEditorTerrainTileCommand
//

interface UUID("006903F2-48A1-4e92-9AC3-DF2D29989E56") IEditorTerrainTileCommand : IEditorCommand
{
   virtual tResult GetTile(uint * pTile) = 0;

   /// @brief Used to identify commands issued as part of the same drag operation, for example
   virtual tResult GetStamp(ulong * pStamp) = 0;
};


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

   virtual tResult OnKeyDown(const cEditorKeyEvent & keyEvent, IEditorView * pView) = 0;
   virtual tResult OnKeyUp(const cEditorKeyEvent & keyEvent, IEditorView * pView) = 0;
   virtual tResult OnLButtonDblClk(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;
   virtual tResult OnLButtonDown(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;
	virtual tResult OnLButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;
   virtual tResult OnRButtonDblClk(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;
	virtual tResult OnRButtonDown(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;
	virtual tResult OnRButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;
	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;
   virtual tResult OnMouseWheel(const cEditorMouseWheelEvent & mouseWheelEvent, IEditorView * pView) = 0;
};

////////////////////////////////////////

class cDefaultEditorTool : public IEditorTool
{
public:
   virtual tResult Activate() { return S_OK; }
   virtual tResult Deactivate() { return S_OK; }

   virtual tResult OnKeyDown(const cEditorKeyEvent & keyEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnKeyUp(const cEditorKeyEvent & keyEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnLButtonDblClk(const cEditorMouseEvent & mouseEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnLButtonDown(const cEditorMouseEvent & mouseEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
	virtual tResult OnLButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnRButtonDblClk(const cEditorMouseEvent & mouseEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
	virtual tResult OnRButtonDown(const cEditorMouseEvent & mouseEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
	virtual tResult OnRButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
   virtual tResult OnMouseWheel(const cEditorMouseWheelEvent & mouseWheelEvent, IEditorView * pView) { return S_EDITOR_TOOL_CONTINUE; }
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORAPI_H
