/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORTILE_H)
#define INCLUDED_EDITORTILE_H

#include "editorapi.h"

#include "str.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS cEditorTile
//

class cEditorTile : public cComObject<IMPLEMENTS(IEditorTile)>
{
public:
   cEditorTile(const tChar * pszName, const tChar * pszTexture, int horzImages, int vertImages);
   ~cEditorTile();

   virtual tResult GetName(cStr * pName) const;
   virtual tResult GetTexture(cStr * pTexture) const;

   virtual uint GetHorizontalImageCount() const;
   virtual uint GetVerticalImageCount() const;

private:
   cStr m_name, m_texture;
   int m_horzImages, m_vertImages;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTILE_H)
