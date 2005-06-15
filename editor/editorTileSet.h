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
   cEditorTileSet(const tChar * pszName, const std::vector<cStr> & textures);
   ~cEditorTileSet();

   virtual tResult GetName(cStr * pName) const;

   virtual tResult GetTileCount(uint * pTileCount) const;
   virtual tResult GetTileTexture(uint iTile, cStr * pTexture) const;
   virtual tResult GetTileName(uint iTile, cStr * pName) const;

   virtual tResult GetImageList(uint dimension, HIMAGELIST * phImageList);

private:
   cStr m_name;
   std::vector<cStr> m_textures;

   typedef std::map<uint, HIMAGELIST> tImageLists;
   tImageLists m_imageLists;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTILESET_H)
