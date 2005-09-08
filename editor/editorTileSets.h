///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORTILESETS_H
#define INCLUDED_EDITORTILESETS_H

#include "editorapi.h"
#include "connptimpl.h"
#include "globalobjdef.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorTileSets
//

class cEditorTileSets : public cComObject2<IMPLEMENTSCP(IEditorTileSets, IEditorTileSetsListener), IMPLEMENTS(IGlobalObject)>
{
public:
   cEditorTileSets();
   ~cEditorTileSets();

   DECLARE_NAME(EditorTileSets)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult AddTileSet(const tChar * pszTileSet);

   virtual tResult GetTileSetCount(uint * pTileSetCount) const;
   virtual tResult GetTileSet(uint index, cStr * pTileSet) const;

   virtual tResult SetDefaultTileSet(const tChar * pszTileSet);
   virtual tResult GetDefaultTileSet(cStr * pTileSet) const;

protected:
   bool HasTileSet(const tChar * pszTileSet) const;

private:
   typedef std::vector<cStr> tStrings;
   tStrings m_tileSets;

   cStr m_defaultTileSet;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORTILESETS_H
