/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORCOMMANDS_H)
#define INCLUDED_EDITORCOMMANDS_H

#include "editorapi.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

F_DECLARE_INTERFACE(ITerrainModel);

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileCommand
//

class cTerrainTileCommand : public cComObject<IMPLEMENTS(IEditorTerrainTileCommand)>
{
public:
   cTerrainTileCommand(ITerrainModel * pTerrain, uint ix, uint iz, uint tile, ulong stamp);
   ~cTerrainTileCommand();

   /////////////////////////////////////

   virtual tResult Do();
   virtual tResult CanUndo();
   virtual tResult Undo();
   virtual tResult GetLabel(cStr * pLabel);
   virtual tResult Compare(IEditorCommand * pOther);

   /////////////////////////////////////

   virtual tResult GetTile(uint * pTile);
   virtual tResult GetStamp(ulong * pStamp);

private:
   cAutoIPtr<ITerrainModel> m_pModel;
   uint m_ix, m_iz, m_tile, m_oldTile;
   ulong m_stamp; // used to identify commands issued as part of the same drag operation
};


/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORCOMMANDS_H)
