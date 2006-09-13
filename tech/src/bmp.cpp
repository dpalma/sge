///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "imageapi.h"
#include "readwriteapi.h"

#include <cstring> // required w/ gcc for memcpy

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

static const uint16 kBmpFileId = 0x4D42; // bfType member of file header must be this value

struct sBmpPaletteEntry
{
   byte peBlue;
   byte peGreen;
   byte peRed;
   byte peFlags;
};

#pragma pack(push,2)
struct sBmpFileHeader
{
   uint16 bfType;
   uint32 bfSize;
   uint16 bfReserved1;
   uint16 bfReserved2;
   uint32 bfOffBits;
};
#pragma pack(pop)

struct sBmpInfoHeader
{
   uint32 biSize;
   long biWidth;
   long biHeight;
   uint16 biPlanes;
   uint16 biBitCount;
   uint32 biCompression;
   uint32 biSizeImage;
   long biXPelsPerMeter;
   long biYPelsPerMeter;
   uint32 biClrUsed;
   uint32 biClrImportant;
};

enum eBmpCompression
{
   BI_RGB        = 0L,
   BI_RLE8       = 1L,
   BI_RLE4       = 2L,
   BI_BITFIELDS  = 3L,
   BI_JPEG       = 4L,
   BI_PNG        = 5L,
};

////////////////////////////////////////////////////////////////////////////////

static IImage * LoadBmp24Bit(IReader * pReader,
                             const sBmpFileHeader & header,
                             const sBmpInfoHeader & info)
{
   Assert(info.biBitCount == 24);

   if (pReader->Seek(header.bfOffBits, kSO_Set) != S_OK)
   {
      return NULL;
   }

   size_t memSize = header.bfSize - header.bfOffBits;
   byte * pImageData = new byte[memSize];
   if (pImageData == NULL)
   {
      return NULL;
   }

   size_t unalignedMemSize = info.biWidth * info.biHeight * info.biBitCount / 8;
   cAutoIPtr<IImage> pImage;

   if (memSize == unalignedMemSize)
   {
      if (pReader->Read(pImageData, memSize) == S_OK)
      {
         ImageCreate(info.biWidth, info.biHeight, kPF_BGR888, pImageData, &pImage);
      }
   }
   else
   {
      size_t unalignedScanLine = unalignedMemSize / info.biHeight;
      byte * p = pImageData;
      for (int i = 0; i < info.biHeight; i++, p += unalignedScanLine)
      {
         if (pReader->Read(p, unalignedScanLine) != S_OK)
         {
            return NULL;
         }
      }

      ImageCreate(info.biWidth, info.biHeight, kPF_BGR888, pImageData, &pImage);
   }

   delete [] pImageData;

   return !pImage ? NULL : CTAddRef(pImage);
}

////////////////////////////////////////////////////////////////////////////////

static IImage * LoadBmp8BitAs24(IReader * pReader,
                                const sBmpFileHeader & header,
                                const sBmpInfoHeader & info)
{
   Assert(pReader != NULL);
   Assert(info.biBitCount == 8);

   uint nColors = (info.biClrUsed > 0) ? info.biClrUsed : (1 << info.biBitCount);
   if (nColors == 0)
   {
      return NULL;
   }

   sBmpPaletteEntry * pPaletteEntries = new sBmpPaletteEntry[nColors];
   if (pPaletteEntries == NULL)
   {
      return NULL;
   }

   cAutoIPtr<IImage> pImage;

   uint nRead;
   if (pReader->Read(pPaletteEntries, nColors * sizeof(sBmpPaletteEntry), &nRead) == S_OK)
   {
      ulong palIndexDataSize = header.bfSize - header.bfOffBits;
      byte * pPalIndexData = new byte[palIndexDataSize];
      if (pPalIndexData != NULL)
      {
         if (pReader->Read(pPalIndexData, palIndexDataSize, &nRead) == S_OK)
         {
            size_t imageSize24 = info.biWidth * info.biHeight * 3;

            byte * pImageBits24 = new byte[imageSize24];
            if (pImageBits24 != NULL)
            {
               byte * pPixel24 = pImageBits24;

               for (uint i = 0; i < palIndexDataSize; i++, pPixel24 += 3)
               {
                  byte palIndex = pPalIndexData[i];

                  pPixel24[0] = pPaletteEntries[palIndex].peBlue;
                  pPixel24[1] = pPaletteEntries[palIndex].peGreen;
                  pPixel24[2] = pPaletteEntries[palIndex].peRed;
               }

               ImageCreate(info.biWidth, info.biHeight, kPF_BGR888, pImageBits24, &pImage);

               delete [] pImageBits24;
            }
         }

         delete [] pPalIndexData;
      }
   }

   delete [] pPaletteEntries;

   return !pImage ? NULL : CTAddRef(pImage);
}

////////////////////////////////////////////////////////////////////////////////

void * BmpLoad(IReader * pReader)
{
   Assert(pReader != NULL);

   sBmpFileHeader header;
   sBmpInfoHeader info;

   if (pReader->Read(&header, sizeof(header)) != S_OK ||
       header.bfType != kBmpFileId ||
       pReader->Read(&info, sizeof(info)) != S_OK)
   {
      return NULL;
   }

   if (info.biBitCount == 24)
   {
      return LoadBmp24Bit(pReader, header, info);
   }
   else if (info.biBitCount == 8)
   {
      return LoadBmp8BitAs24(pReader, header, info);
   }
   else
   {
      DebugMsg1("Un-supported BMP image format (%d bits per pixel)\n", info.biBitCount);
      return NULL;
   }
}


///////////////////////////////////////////////////////////////////////////////

tResult BmpWrite(IImage * pImage, IWriter * pWriter)
{
   if (pImage == NULL || pWriter == NULL)
   {
      return E_POINTER;
   }

   ePixelFormat pixelFormat = pImage->GetPixelFormat();

   if (pixelFormat != kPF_RGB888 && pixelFormat != kPF_BGR888
      && pixelFormat != kPF_RGBA8888 && pixelFormat != kPF_BGRA8888)
   {
      return E_INVALIDARG;
   }

   // BMP file scan lines must be 4-byte aligned
   int scanLineWidth = ((pImage->GetWidth() * BytesPerPixel(pixelFormat)) + 3) & ~3;
   int bitsSize = abs(scanLineWidth * pImage->GetHeight());

   sBmpFileHeader header;
   header.bfType = kBmpFileId;
   header.bfReserved1 = 0;
   header.bfReserved2 = 0;
   header.bfOffBits = sizeof(sBmpFileHeader) + sizeof(sBmpInfoHeader);
   header.bfSize = sizeof(sBmpFileHeader) + sizeof(sBmpInfoHeader) + bitsSize;

   sBmpInfoHeader info;
   info.biSize = sizeof(sBmpInfoHeader);
   info.biWidth = pImage->GetWidth();
   info.biHeight = -static_cast<int>(pImage->GetHeight());
   info.biPlanes = 1;
   info.biCompression = BI_RGB;
   info.biBitCount = BytesPerPixel(pixelFormat) * 8;

   if (pWriter->Write(&header, sizeof(sBmpFileHeader)) == S_OK
      && pWriter->Write(&info, sizeof(sBmpInfoHeader)) == S_OK)
   {
      if (info.biBitCount == 24)
      {
         uint bytesPerPixel = BytesPerPixel(pixelFormat);
         uint srcScanLineWidth = pImage->GetWidth() * bytesPerPixel;
         const byte * pSrcData = static_cast<const byte *>(pImage->GetData());
         for (uint i = 0; i < pImage->GetHeight(); i++)
         {
            if (pixelFormat == kPF_BGR888)
            {
               if (pWriter->Write(const_cast<byte*>(pSrcData), scanLineWidth) != S_OK)
               {
                  return E_FAIL;
               }
            }
            else if (pixelFormat == kPF_RGB888)
            {
               for (uint j = 0; j < pImage->GetWidth(); j++)
               {
                  const byte * pPixel = pSrcData + (j * bytesPerPixel);
                  byte bgr[3] = { pPixel[2], pPixel[1], pPixel[0] };
                  if (pWriter->Write(bgr, sizeof(bgr)) != S_OK)
                  {
                     return E_FAIL;
                  }
               }
            }
            pSrcData += srcScanLineWidth;
         }
         return S_OK;
      }
      else
      {
         if (pixelFormat == kPF_BGRA8888)
         {
            if (pWriter->Write(const_cast<void*>(pImage->GetData()), bitsSize) == S_OK)
            {
               return S_OK;
            }
         }
         else if (pixelFormat == kPF_RGBA8888)
         {
            uint bytesPerPixel = BytesPerPixel(pixelFormat);
            uint srcScanLineWidth = pImage->GetWidth() * bytesPerPixel;
            const byte * pSrcData = static_cast<const byte *>(pImage->GetData());
            for (uint i = 0; i < pImage->GetHeight(); i++)
            {
               for (uint j = 0; j < pImage->GetWidth(); j++)
               {
                  const byte * pPixel = pSrcData + (j * bytesPerPixel);
                  byte bgr[4] = { pPixel[2], pPixel[1], pPixel[0], pPixel[3] };
                  if (pWriter->Write(bgr, sizeof(bgr)) != S_OK)
                  {
                     return E_FAIL;
                  }
               }
               pSrcData += srcScanLineWidth;
            }
            return S_OK;
         }
      }
   }


   return E_FAIL;
}


////////////////////////////////////////////////////////////////////////////////
