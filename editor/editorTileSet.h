/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORTILESET_H)
#define INCLUDED_EDITORTILESET_H

#include "editorapi.h"

#include "comtools.h"
#include "str.h"

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

   virtual tResult AddTileTexture(const tChar * pszName,
                                  const tChar * pszTexture,
                                  int horzImages,
                                  int vertImages);

private:
   cStr m_name;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTILESET_H)
