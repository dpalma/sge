///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "imagedata.h"
#include "readwriteapi.h"

#include <cstring> // required w/ gcc for memcpy

#include "dbgalloc.h" // must be last header

struct sBmpPaletteEntry
{
   byte peRed;
   byte peGreen;
   byte peBlue;
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

cImageData * LoadBmp(IReader * pReader)
{
   Assert(pReader != NULL);

   sBmpFileHeader header;
   sBmpInfoHeader info;

   if (pReader->Read(&header, sizeof(header)) != S_OK ||
       header.bfType != 0x4D42 ||
       pReader->Read(&info, sizeof(info)) != S_OK)
   {
      return NULL;
   }

   if (info.biBitCount != 24)
   {
      DebugMsg1("Un-supported BMP image format (%d bits per pixel)\n", info.biBitCount);
      return NULL;
   }

   cImageData * pImage = NULL;

   ulong memSize = header.bfSize - header.bfOffBits;

   byte * pAlignedImageBits = new byte[memSize];

   if (pAlignedImageBits != NULL)
   {
      // The image scan-lines image in the file will be aligned on 4-byte
      // boundaries. Do work here to remove the padding. Will mean more work
      // later if this image data will be used to create Windows HBITMAPs,
      // but will make it consistent with the Targa loader and the image client
      // code that has been written thus far.
      if (pReader->Seek(header.bfOffBits, kSO_Set) == S_OK
         && pReader->Read(pAlignedImageBits, memSize) == S_OK)
      {
         size_t unalignedMemSize = info.biWidth * info.biHeight * info.biBitCount / 8;

         byte * pUnalignedImageBits = new byte[unalignedMemSize];

         if (pUnalignedImageBits != NULL)
         {
            uint bytesPerPixel = 3; // only support 24 bit (24/8 = 3)

            size_t unalignedScanLine = info.biWidth * bytesPerPixel;
            size_t alignedScanLine = memSize / info.biHeight;

            if (unalignedScanLine == alignedScanLine)
            {
               pImage = new cImageData;
               if (!pImage->Create(info.biWidth, info.biHeight, kPF_BGR888, pAlignedImageBits))
               {
                  delete pImage;
                  pImage = NULL;
               }
            }
            else
            {
               byte * pAligned = pAlignedImageBits;
               byte * pUnaligned = pUnalignedImageBits;

               for (int i = 0; i < info.biHeight; i++)
               {
                  memcpy(pUnaligned, pAligned, unalignedScanLine);
                  pUnaligned += unalignedScanLine;
                  pAligned += alignedScanLine;
               }

               pImage = new cImageData;
               if (!pImage->Create(info.biWidth, info.biHeight, kPF_BGR888, pUnalignedImageBits))
               {
                  delete pImage;
                  pImage = NULL;
               }
            }

            delete [] pUnalignedImageBits;
         }
      }

      delete [] pAlignedImageBits;
   }

   return pImage;
}
