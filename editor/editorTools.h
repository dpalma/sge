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

F_DECLARE_INTERFACE(IEntity);


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

private:
   cAutoIPtr<IEditorView> m_pView;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSelectTool
//

class cSelectTool : public cComObject<cDragTool, &IID_IEditorTool>
{
public:
   cSelectTool();
   ~cSelectTool();

   virtual tResult OnKeyDown(const cEditorKeyEvent & keyEvent, IEditorView * pView);
   virtual tResult GetToolTip(const cEditorMouseEvent & mouseEvent, cStr * pToolTipText, uint_ptr * pToolTipId) const;

protected:
   virtual tResult OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

private:
   tResult GetRayHitEntity(const cEditorMouseEvent & mouseEvent, IEntity * * ppEntity) const;

   enum eSelectType { kST_Box, kST_Ray };
   eSelectType m_selectType;
   CPoint m_dragStartPoint, m_lastMousePoint;
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
   virtual tResult GetToolTip(const cEditorMouseEvent & mouseEvent, cStr * pToolTipText, uint_ptr * pToolTipId) const;

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
// CLASS: cPlaceEntityTool
//

class cPlaceEntityTool : public cComObject<cDragTool, &IID_IEditorTool>
{
public:
   cPlaceEntityTool(const cStr & model);
   ~cPlaceEntityTool();

protected:
   virtual tResult OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

private:
   cStr m_model;
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

   virtual tResult Activate();
   virtual tResult Deactivate();

protected:
   static bool GetHitQuad(CPoint point, IEditorView * pView, HTERRAINQUAD * phQuad);
   static bool GetHitVertex(CPoint point, IEditorView * pView, HTERRAINVERTEX * phVertex);
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileTool
//

class cTerrainTileTool : public cComObject<cTerrainTool, &IID_IEditorTool>
{
public:
   cTerrainTileTool(uint tile);
   ~cTerrainTileTool();

	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

   virtual tResult GetToolTip(const cEditorMouseEvent & mouseEvent, cStr * pToolTipText, uint_ptr * pToolTipId) const;

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
//
// CLASS: cTerrainElevationTool
//

class cTerrainElevationTool : public cComObject<cTerrainTool, &IID_IEditorTool>
{
public:
   cTerrainElevationTool();
   ~cTerrainElevationTool();

	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

   virtual tResult GetToolTip(const cEditorMouseEvent & mouseEvent, cStr * pToolTipText, uint_ptr * pToolTipId) const;

protected:
   virtual tResult OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

private:
   cAutoIPtr<IEditorCommand> m_pCommand;
   HTERRAINVERTEX m_hHitVertex;
   CPoint m_lastDragPoint;
   float m_elevDelta;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainPlateauTool
//

class cTerrainPlateauTool : public cComObject<cTerrainTool, &IID_IEditorTool>
{
public:
   cTerrainPlateauTool();
   ~cTerrainPlateauTool();

	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

protected:
   virtual tResult OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
   virtual tResult OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

private:
   float m_elevation;
   cAutoIPtr<IEditorCompositeCommand> m_pCommand;
   std::set<HTERRAINVERTEX> m_hitVertices;
};


/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTOOLS_H)
