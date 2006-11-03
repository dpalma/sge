//////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_IMAGE_H
#define INCLUDED_IMAGE_H

#include "tech/imageapi.h"

#ifdef _MSC_VER
#pragma once
#endif


//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageBase
//

class cImageBase : public IImage
{
public:
   cImageBase();
   virtual ~cImageBase();

   virtual uint GetWidth() const;
   virtual uint GetHeight() const;
   virtual ePixelFormat GetPixelFormat() const;
   virtual const void * GetData() const;

   virtual tResult Clone(IImage * * ppImage);

protected:
   inline void Construct(uint width, uint height, ePixelFormat pixelFormat, byte * pData)
   {
      m_width = width;
      m_height = height;
      m_pixelFormat = pixelFormat;
      m_pData = pData;
   }
   inline byte * GetDataBytes() { return m_pData; }
   inline const byte * GetDataBytes() const { return m_pData; }

private:
   uint m_width, m_height;
   ePixelFormat m_pixelFormat;
   byte * m_pData;
};


//////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cImageT
//

template <typename PIXEL>
class cImageT : public cComObject<cImageBase, &IID_IImage>
{
   cImageT(const cImageT &);
   const cImageT & operator =(const cImageT &);

public:
   cImageT(uint width, uint height, ePixelFormat pixelFormat, byte * pData);
   ~cImageT();

   virtual tResult GetPixel(uint x, uint y, byte rgba[4]) const;
   virtual tResult SetPixel(uint x, uint y, const byte rgba[4]);

   virtual tResult GetSubImage(uint x, uint y, uint width, uint height, IImage * * ppSubImage) const;
};

////////////////////////////////////////

template <typename PIXEL>
cImageT<PIXEL>::cImageT(uint width, uint height, ePixelFormat pixelFormat, byte * pData)
{
   cImageBase::Construct(width, height, pixelFormat, pData);
}

////////////////////////////////////////

template <typename PIXEL>
cImageT<PIXEL>::~cImageT()
{
}

////////////////////////////////////////

template <typename PIXEL>
tResult cImageT<PIXEL>::GetPixel(uint x, uint y, byte rgba[4]) const
{
   if (x >= GetWidth() || y >= GetHeight())
   {
      return E_INVALIDARG;
   }

   if (rgba == NULL)
   {
      return E_POINTER;
   }

   uint bytesPerPixel = PIXEL::BytesPerPixel();
   const byte * pPixel = GetDataBytes() + ((y * GetWidth()) + x) * bytesPerPixel;
   PIXEL::GetPixel(pPixel, rgba);
   return S_OK;
}

////////////////////////////////////////

template <typename PIXEL>
tResult cImageT<PIXEL>::SetPixel(uint x, uint y, const byte rgba[4])
{
   if (x >= GetWidth() || y >= GetHeight())
   {
      return E_INVALIDARG;
   }

   if (rgba == NULL)
   {
      return E_POINTER;
   }

   uint bytesPerPixel = PIXEL::BytesPerPixel();
   byte * pPixel = GetDataBytes() + ((y * GetWidth()) + x) * bytesPerPixel;
   PIXEL::SetPixel(pPixel, rgba);
   return S_OK;
}

////////////////////////////////////////

template <typename PIXEL>
tResult cImageT<PIXEL>::GetSubImage(uint x, uint y, uint width, uint height, IImage * * ppSubImage) const
{
   if (width == 0 || height == 0)
   {
      return E_INVALIDARG;
   }

   if (ppSubImage == NULL)
   {
      return E_POINTER;
   }

   uint bytesPerPixel = PIXEL::BytesPerPixel();

   uint memSize = bytesPerPixel * width * height;
   if (memSize == 0)
   {
      WarnMsg1("Invalid pixel format %d\n", GetPixelFormat());
      return E_FAIL;
   }

   byte * pImageData = new byte[memSize];
   if (pImageData == NULL)
   {
      return E_OUTOFMEMORY;
   }

   uint scanLine = bytesPerPixel * width;
   uint scanLine2 = bytesPerPixel * GetWidth();

   byte * p = pImageData;
   const byte * p2 = GetDataBytes() + (scanLine2 * y) + x;

   for (uint i = 0; i < height; i++)
   {
      memcpy(p, p2, scanLine);
      p += scanLine;
      p2 += scanLine2;
   }

   cAutoIPtr<IImage> pSubImage(static_cast<IImage*>(new cImageT<PIXEL>(width, height, GetPixelFormat(), pImageData)));
   if (!pSubImage)
   {
      return E_OUTOFMEMORY;
   }

   return pSubImage.GetPointer(ppSubImage);
}


//////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_IMAGE_H

