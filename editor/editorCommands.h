/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORCOMMANDS_H)
#define INCLUDED_EDITORCOMMANDS_H

#include "editorapi.h"

#include <list>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


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
   cTerrainTileCommand(uint ix, uint iz, uint tile);
   ~cTerrainTileCommand();

   /////////////////////////////////////

   virtual tResult Do();
   virtual tResult CanUndo();
   virtual tResult Undo();
   virtual tResult GetLabel(cStr * pLabel);

private:
   uint m_ix, m_iz, m_tile, m_oldTile;
};


/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORCOMMANDS_H)
