/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORTILE_H)
#define INCLUDED_EDITORTILE_H

#include "editorapi.h"

#include "textureapi.h"

#include "comtools.h"
#include "str.h"

#include <map>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cImageData;

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

   virtual tResult GetTexture(ITexture * * ppTexture);

   virtual tResult GetBitmap(uint dimension, bool bEntire, HBITMAP * phBitmap);

private:
   void LazyInit();

   cStr m_name, m_texture;
   int m_horzImages, m_vertImages;

   cImageData * m_pImageData;

   cAutoIPtr<ITexture> m_pTexture;

   HBITMAP m_hBitmap;

   bool m_bLoadBitmapFailed;

   typedef std::map<uint, HBITMAP> tBitmaps;
   tBitmaps m_bitmaps;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTILE_H)
