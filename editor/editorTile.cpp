///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTile.h"

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS cEditorTile
//

///////////////////////////////////////

cEditorTile::cEditorTile(const tChar * pszName, const tChar * pszTexture,
                         int horzImages, int vertImages)
 : m_name(pszName != NULL ? pszName : ""),
   m_texture(pszTexture != NULL ? pszTexture : ""),
   m_horzImages(horzImages),
   m_vertImages(vertImages)
{
}

///////////////////////////////////////

cEditorTile::~cEditorTile()
{
}

///////////////////////////////////////

tResult cEditorTile::GetName(cStr * pName) const
{
   if (pName != NULL)
   {
      *pName = m_name;
      return S_OK;
   }
   else
   {
      return E_POINTER;
   }
}

///////////////////////////////////////

tResult cEditorTile::GetTexture(cStr * pTexture) const
{
   if (pTexture != NULL)
   {
      *pTexture = m_texture;
      return S_OK;
   }
   else
   {
      return E_POINTER;
   }
}

///////////////////////////////////////

tResult cEditorTile::GetButtonImage(HBITMAP * phBitmap)
{
   return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
