/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORTOOLS_H)
#define INCLUDED_EDITORTOOLS_H

#include "editorapi.h"

#include <set>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_HANDLE(HTERRAINQUAD);


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDragTool
//

class cDragTool : public cDefaultEditorTool
{
public:
   cDragTool();
   virtual ~cDragTool() = 0;

   virtual tResult OnKeyDown(const cEditorKeyEvent & keyEvent, IEditorView * pView);
   virtual tResult OnLButtonDown(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
	virtual tResult OnLButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

protected:
   virtual tResult OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;
   virtual tResult OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;
   virtual tResult OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView) = 0;

   bool IsDragging();

   IEditorView * AccessView();

   ulong GetNextStamp();

private:
   cAutoIPtr<IEditorView> m_pView;
   ulong m_nextStamp;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMoveCameraTool
//

class cMoveCameraTool : public cComObject<cDragTool, &IID_IEditorTool>
{
public:
   cMoveCameraTool();
   ~cMoveCameraTool();

   virtual tResult OnKeyDown(const cEditorKeyEvent & keyEvent, IEditorView * pView);
   virtual tResult OnMouseWheel(const cEditorMouseWheelEvent & mouseWheelEvent, IEditorView * pView);

protected:
   virtual tResult OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

   void MoveCamera(IEditorView * pView, CPoint delta);

private:
   CPoint m_lastMousePoint;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTool
//

class cTerrainTool : public cDragTool
{
public:
   cTerrainTool();
   ~cTerrainTool();

protected:
   static bool GetHitQuad(CPoint point, IEditorView * pView, HTERRAINQUAD * phQuad);
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileTool
//

class cTerrainTileTool : public cComObject<cTerrainTool, &IID_IEditorTool>
{
public:
   cTerrainTileTool();
   ~cTerrainTileTool();

   void SetTile(uint tile);

   virtual tResult Activate();
   virtual tResult Deactivate();

	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

protected:
   virtual tResult OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

private:
   uint m_tile;
   cAutoIPtr<IEditorCompositeCommand> m_pCommand;
   std::set<HTERRAINQUAD> m_hitQuads;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTOOLS_H)
