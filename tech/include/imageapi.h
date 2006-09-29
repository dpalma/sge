//////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_IMAGEAPI_H
#define INCLUDED_IMAGEAPI_H

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IImage);
F_DECLARE_INTERFACE(IWriter);

F_DECLARE_HANDLE(HBITMAP);


//////////////////////////////////////////////////////////////////////////////

TECH_API tResult BmpWrite(IImage * pImage, IWriter * pWriter);

TECH_API tResult ImageToWindowsBitmap(IImage * pImage, HBITMAP * phBitmap);


//////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IImage
//

////////////////////////////////////////

enum ePixelFormat
{
   kPF_ERROR = -1,

   // 8-bit
   kPF_Grayscale,
   kPF_ColorMapped,

   // 16-bit
   kPF_RGB555,
   kPF_BGR555,
   kPF_RGB565,
   kPF_BGR565,
   kPF_RGBA1555,
   kPF_BGRA1555,

   // 24-bit
   kPF_RGB888,
   kPF_BGR888,

   // 32-bit
   kPF_RGBA8888,
   kPF_BGRA8888,

   kPF_NumPixelFormats, // must be the last member of the enumeration
};

TECH_API uint BytesPerPixel(ePixelFormat pixelFormat);

////////////////////////////////////////

interface IImage : IUnknown
{
   virtual uint GetWidth() const = 0;
   virtual uint GetHeight() const = 0;
   virtual ePixelFormat GetPixelFormat() const = 0;
   virtual const void * GetData() const = 0;

   virtual tResult GetPixel(uint x, uint y, byte rgba[4]) const = 0;
   virtual tResult SetPixel(uint x, uint y, const byte rgba[4]) = 0;

   virtual tResult GetSubImage(uint x, uint y, uint width, uint height, IImage * * ppSubImage) const = 0;

   virtual tResult Clone(IImage * * ppImage) = 0;
};

////////////////////////////////////////

#define kRT_Image          _T("Image")          // Resource type that returns an IImage*
#define kRT_WindowsDDB     _T("WindowsDDB")     // Resource type that returns an HBITMAP

TECH_API tResult ImageRegisterResourceFormats();


//////////////////////////////////////////////////////////////////////////////

TECH_API tResult ImageCreate(uint width, uint height, ePixelFormat pixelFormat, const void * pData, IImage * * ppImage);


//////////////////////////////////////////////////////////////////////////////

TECH_API void ImageApplyGamma(IImage * pImage, uint x, uint y, uint w, uint h, float gamma);


//////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_IMAGEAPI_H
