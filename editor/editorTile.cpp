///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

uint cEditorTile::GetHorizontalImageCount() const
{
   return m_horzImages;
}

///////////////////////////////////////

uint cEditorTile::GetVerticalImageCount() const
{
   return m_vertImages;
}

///////////////////////////////////////////////////////////////////////////////
