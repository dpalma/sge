///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTileSet.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS cEditorTileSet
//

///////////////////////////////////////

cEditorTileSet::cEditorTileSet(const tChar * pszName)
 : m_name(pszName != NULL ? pszName : "")
{
}

///////////////////////////////////////

cEditorTileSet::~cEditorTileSet()
{
}

///////////////////////////////////////

tResult cEditorTileSet::AddTileTexture(const tChar * pszName,
                                       const tChar * pszTexture,
                                       int horzImages,
                                       int vertImages)
{
   return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
