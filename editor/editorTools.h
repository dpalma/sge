/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORTOOLS_H)
#define INCLUDED_EDITORTOOLS_H

#include "editorapi.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMoveCameraTool
//

class cMoveCameraTool : public cComObject<cDefaultEditorTool, &IID_IEditorTool>
{
public:
   cMoveCameraTool();
   ~cMoveCameraTool();

   virtual tResult OnLButtonDown(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
	virtual tResult OnLButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

private:
   cAutoIPtr<IEditorView> m_pView;
   CPoint m_lastMousePoint;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileTool
//

class cTerrainTileTool : public cComObject<cDefaultEditorTool, &IID_IEditorTool>
{
public:
   cTerrainTileTool();
   ~cTerrainTileTool();

   void SetTile(uint tile);

   virtual tResult OnLButtonDown(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
	virtual tResult OnLButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView);
	virtual tResult OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView);

private:
   uint m_tile;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTOOLS_H)
