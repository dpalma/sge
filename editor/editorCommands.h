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

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorCompositeCommand
//

class cEditorCompositeCommand : public cComObject<IMPLEMENTS(IEditorCompositeCommand)>
{
public:
   cEditorCompositeCommand();
   ~cEditorCompositeCommand();

   virtual tResult Do();
   virtual tResult CanUndo();
   virtual tResult Undo();
   virtual tResult GetLabel(cStr * pLabel);

   virtual tResult Add(IEditorCommand * pCommand);
   virtual tResult Remove(IEditorCommand * pCommand);

private:
   typedef std::list<IEditorCommand *> tCmds;
   tCmds m_cmds;
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

#endif // !defined(INCLUDED_EDITORCOMMANDS_H)
