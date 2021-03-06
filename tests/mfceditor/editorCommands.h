/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORCOMMANDS_H)
#define INCLUDED_EDITORCOMMANDS_H

#include "editorapi.h"

#include <list>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_HANDLE(HTERRAINQUAD);
F_DECLARE_HANDLE(HTERRAINVERTEX);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorCompositeCommand
//

class cEditorCompositeCommand : public cComObject<IMPLEMENTS(IEditorCompositeCommand)>
{
public:
   cEditorCompositeCommand(tEditorCompositeCommandCallback pfnCallback);
   ~cEditorCompositeCommand();

   virtual tResult Do();
   virtual tResult CanUndo();
   virtual tResult Undo();
   virtual tResult GetLabel(cStr * pLabel);

   virtual tResult Add(IEditorCommand * pCommand);
   virtual tResult Remove(IEditorCommand * pCommand);

private:
   void DoCallback(eEditorCompositeCommandCallback type);

   typedef std::list<IEditorCommand *> tCmds;
   tCmds m_cmds;

   tEditorCompositeCommandCallback m_pfnCallback;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileCommand
//

class cTerrainTileCommand : public cComObject<IMPLEMENTS(IEditorCommand)>
{
public:
   cTerrainTileCommand(HTERRAINQUAD hQuad, uint tile);
   ~cTerrainTileCommand();

   virtual tResult Do();
   virtual tResult CanUndo();
   virtual tResult Undo();
   virtual tResult GetLabel(cStr * pLabel);

private:
   HTERRAINQUAD m_hQuad;
   uint m_tile, m_oldTile;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChangeElevationCommand
//

class cTerrainChangeElevationCommand : public cComObject<IMPLEMENTS(IEditorCommand)>
{
public:
   cTerrainChangeElevationCommand(HTERRAINVERTEX hVertex, float elevDelta);
   ~cTerrainChangeElevationCommand();

   virtual tResult Do();
   virtual tResult CanUndo();
   virtual tResult Undo();
   virtual tResult GetLabel(cStr * pLabel);

private:
   HTERRAINVERTEX m_hVertex;
   float m_elevDelta, m_oldElevation;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainSetElevationCommand
//

class cTerrainSetElevationCommand : public cComObject<IMPLEMENTS(IEditorCommand)>
{
public:
   cTerrainSetElevationCommand(HTERRAINVERTEX hVertex, float elevation, uint labelId);
   ~cTerrainSetElevationCommand();

   virtual tResult Do();
   virtual tResult CanUndo();
   virtual tResult Undo();
   virtual tResult GetLabel(cStr * pLabel);

private:
   HTERRAINVERTEX m_hVertex;
   float m_elevation, m_oldElevation;
   uint m_labelId;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPlaceEntityCommand
//

class cPlaceEntityCommand : public cComObject<IMPLEMENTS(IEditorCommand)>
{
public:
   cPlaceEntityCommand(const tChar * pszEntity, const tVec3 & position);
   ~cPlaceEntityCommand();

   virtual tResult Do();
   virtual tResult CanUndo();
   virtual tResult Undo();
   virtual tResult GetLabel(cStr * pLabel);

private:
   cStr m_entity;
   tVec3 m_position;
};


/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORCOMMANDS_H)
