///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "imagedata.h"
#include "readwriteapi.h"
#include "resourceapi.h"

#include <cstring> // required w/ gcc for memcpy

#include "dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

static cImageData * LoadBmp24Bit(IReader * pReader,
                                 const sBmpFileHeader & header,
                                 const sBmpInfoHeader & info)
{
   cImageData * pImageData = NULL;

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
               pImageData = new cImageData;
               if (!pImageData->Create(info.biWidth, info.biHeight, kPF_BGR888, pAlignedImageBits))
               {
                  delete pImageData;
                  pImageData = NULL;
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

               pImageData = new cImageData;
               if (!pImageData->Create(info.biWidth, info.biHeight, kPF_BGR888, pUnalignedImageBits))
               {
                  delete pImageData;
                  pImageData = NULL;
               }
            }

            delete [] pUnalignedImageBits;
         }
      }

      delete [] pAlignedImageBits;
   }

   return pImageData;
}

////////////////////////////////////////////////////////////////////////////////

static cImageData * LoadBmp8BitAs24(IReader * pReader,
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

   cImageData * pImageData = NULL;

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

               pImageData = new cImageData;
               if (!pImageData->Create(info.biWidth, info.biHeight, kPF_BGR888, pImageBits24))
               {
                  delete pImageData;
                  pImageData = NULL;
               }

               delete [] pImageBits24;
            }
         }

         delete [] pPalIndexData;
      }
   }

   delete [] pPaletteEntries;

   return pImageData;
}

////////////////////////////////////////////////////////////////////////////////

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
//
// CLASS: cBmpImage
//

class cBmpImage : public cComObject<IMPLEMENTS(IResource)>
{
public:
   cBmpImage(IReader * pReader);

   virtual eResourceClass GetClass() const { return kRC_Image; }

   virtual tResult GetData(void * * ppData);

private:
   cAutoIPtr<IReader> m_pReader;
};

////////////////////////////////////////

cBmpImage::cBmpImage(IReader * pReader)
 : m_pReader(CTAddRef(pReader))
{
}

////////////////////////////////////////

tResult cBmpImage::GetData(void * * ppData)
{
   if (ppData == NULL)
   {
      return E_POINTER;
   }

   if (!m_pReader)
   {
      return E_FAIL;
   }

   *ppData = LoadBmp(m_pReader);

   return (*ppData != NULL) ? S_OK : E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageFormatBmp
//

class cImageFormatBmp : public cComObject<IMPLEMENTS(IResourceFormat)>
{
public:
   virtual tResult Load(const tResKey & key, IReader * pReader, IResource * * ppResource);
};

////////////////////////////////////////

tResult cImageFormatBmp::Load(const tResKey & key, IReader * pReader, IResource * * ppResource)
{
   if (pReader == NULL || ppResource == NULL)
   {
      return E_POINTER;
   }

   cBmpImage * pImg = new cBmpImage(pReader);
   if (pImg == NULL)
   {
      return E_OUTOFMEMORY;
   }

   *ppResource = static_cast<IResource *>(pImg);

   return S_OK;
}

////////////////////////////////////////

AUTOREGISTER_RESOURCEFORMAT(kRC_Image, bmp, cImageFormatBmp);


////////////////////////////////////////////////////////////////////////////////
