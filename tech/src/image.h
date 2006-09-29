//////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_IMAGE_H
#define INCLUDED_IMAGE_H

#include "imageapi.h"

#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImage
//

class cImage : public cComObject<IMPLEMENTS(IImage)>
{
   cImage(const cImage &);
   const cImage & operator =(const cImage &);

public:
   cImage(uint width, uint height, ePixelFormat pixelFormat, byte * pData);
   ~cImage();

   virtual uint GetWidth() const;
   virtual uint GetHeight() const;
   virtual ePixelFormat GetPixelFormat() const;
   virtual const void * GetData() const;

   virtual tResult GetPixel(uint x, uint y, byte rgba[4]) const;
   virtual tResult SetPixel(uint x, uint y, const byte rgba[4]);

   virtual tResult GetSubImage(uint x, uint y, uint width, uint height, IImage * * ppSubImage) const;

   virtual tResult Clone(IImage * * ppImage);

private:
   uint m_width, m_height;
   ePixelFormat m_pixelFormat;
   byte * m_pData;
};

//////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_IMAGE_H

