//////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_IMAGEDATA_H
#define INCLUDED_IMAGEDATA_H

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IWriter);

//////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageData
//

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

class TECH_API cImageData
{
public:
   cImageData();
   ~cImageData();

   bool Create(int width, int height, ePixelFormat pixelFormat, void * pData = NULL);
   void Destroy();

   unsigned GetWidth() const;
   unsigned GetHeight() const;
   ePixelFormat GetPixelFormat() const;
   const void * GetData() const;
   void * GetData();

private:
   ePixelFormat m_pixelFormat;
   unsigned m_width, m_height;
   byte * m_pData;
};

///////////////////////////////////////

inline unsigned cImageData::GetWidth() const
{
   return m_width;
}

///////////////////////////////////////

inline unsigned cImageData::GetHeight() const
{
   return m_height;
}

///////////////////////////////////////

inline ePixelFormat cImageData::GetPixelFormat() const
{
   return m_pixelFormat;
}

///////////////////////////////////////

inline const void * cImageData::GetData() const
{
   return m_pData;
}

///////////////////////////////////////

inline void * cImageData::GetData()
{
   return m_pData;
}

//////////////////////////////////////////////////////////////////////////////

TECH_API tResult BmpWrite(const cImageData * pImageData, IWriter * pWriter);

//////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_IMAGEDATA_H

