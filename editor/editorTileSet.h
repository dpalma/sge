/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORTILESET_H)
#define INCLUDED_EDITORTILESET_H

#include "editorapi.h"

#include <vector>
#include <map>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS cEditorTileSet
//

class cEditorTileSet : public cComObject<IMPLEMENTS(IEditorTileSet)>
{
public:
   cEditorTileSet(const tChar * pszName);
   ~cEditorTileSet();

   virtual tResult GetName(cStr * pName) const;

   virtual tResult AddTile(const tChar * pszName,
                           const tChar * pszTexture,
                           int horzImages,
                           int vertImages);

   virtual tResult GetTileCount(uint * pTileCount) const;
   virtual tResult GetTileTexture(uint iTile, cStr * pTexture) const;
   virtual tResult GetTileName(uint iTile, cStr * pName) const;

   virtual tResult GetImageList(uint dimension, HIMAGELIST * phImageList);

private:
   cStr m_name;

   typedef std::vector<IEditorTile *> tTiles;
   tTiles m_tiles;

   typedef std::map<uint, HIMAGELIST> tImageLists;
   tImageLists m_imageLists;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTILESET_H)
