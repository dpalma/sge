///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "imagedata.h"
#include "readwriteapi.h"

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
      return NULL;

   ulong memSize = header.bfSize - header.bfOffBits;

   byte * pImageData = new byte[memSize];

   pReader->Seek(header.bfOffBits, kSO_Set);

   if (pReader->Read(pImageData, memSize) != S_OK)
   {
      DebugMsg("Failed to read BMP image data\n");
      delete [] pImageData;
      return false;
   }

   if (info.biBitCount != 24)
   {
      DebugMsg1("Un-supported BMP image format (%d bits per pixel)\n", info.biBitCount);
      delete [] pImageData;
      return NULL;
   }

   cImageData * pImage = new cImageData;
   if (!pImage->Create(info.biWidth, info.biHeight, kPF_BGR888, pImageData))
   {
      delete pImage;
      pImage = NULL;
   }

   delete [] pImageData;

   return pImage;
}
