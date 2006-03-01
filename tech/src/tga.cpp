///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "imageapi.h"
#include "readwriteapi.h"

#include <cstdlib>
#include <cstring> // required w/ gcc for memcpy

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

enum eTargaImageType
{
   kTGA_NoImage = 0,          // No image data included.
   kTGA_UncomprColorMap = 1,  // Uncompressed, color-mapped images.
   kTGA_UncomprRGB = 2,       // Uncompressed, RGB images.
   kTGA_UncomprBW = 3,        // Uncompressed, black and white images.
   kTGA_RLEColorMap = 9,      // Runlength encoded color-mapped images.
   kTGA_RLERGB = 10,          // Runlength encoded RGB images.
   kTGA_ComprBW = 11,         // Compressed, black and white images.
   kTGA_ComprColorMap = 32,   // Compressed color-mapped data, using Huffman, Delta, 
                              // and runlength encoding.
   kTGA_ComprColorMap2 = 33   // Compressed color-mapped data, using Huffman, Delta,
                              // and runlength encoding.  4-pass quadtree-type process.
};

#pragma pack(push,1)
struct sTargaHeader
{
   uint8    IDLength;
   uint8    ColorMapType;
   uint8    ImageType;

   // color map specification (5 bytes)
   uint16   ColorMapStart;
   uint16   ColorMapLength;
   uint8    ColorMapEntrySize;

   // image specification (10 bytes)
   uint16   XOrigin;
   uint16   YOrigin;
   uint16   Width;
   uint16   Height;
   uint8    PixelDepth;
   uint8    ImageDescriptor;
};

struct sNewTargaFooter
{
   uint32 extAreaOffset;      // extension area offset
   uint32 devDirOffset;       // developer directory offset
   char szSignature[18];
};
#pragma pack(pop)

AssertOnce(sizeof(sTargaHeader) == 18);
AssertOnce(sizeof(sNewTargaFooter) == 26);

///////////////////////////////////////////////////////////////////////////////

template <>
tResult cReadWriteOps<sTargaHeader>::Read(IReader * pReader, sTargaHeader * pValue)
{
   if (pReader->Read(pValue, sizeof(*pValue)) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

template <>
tResult cReadWriteOps<sNewTargaFooter>::Read(IReader * pReader, sNewTargaFooter * pValue)
{
   ulong pos;
   if (FAILED(pReader->Tell(&pos)) ||
       pReader->Seek(-(int)(sizeof(sNewTargaFooter)), kSO_End) != S_OK)
   {
      return E_FAIL;
   }

   if (pReader->Read(pValue, sizeof(sNewTargaFooter)) != S_OK)
   {
      pReader->Seek(pos, kSO_Set);
      return E_FAIL;
   }

   pReader->Seek(pos, kSO_Set);

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTargaReader
//

class cTargaReader
{
public:
   cTargaReader(IReader * pReader);
   ~cTargaReader();

   const sTargaHeader & GetHeader() const;
   byte * GetImageData();

   bool ReadHeader();
   bool ReadFooter();
   bool ReadColorMap();
   bool ReadImageData();

   IImage * CreateImage();

private:
   inline IReader * AccessReader() { return m_pReader; }

   sTargaHeader m_header;
   sNewTargaFooter m_footer;
   char m_szId[256];
   byte * m_pColorMap;
   cAutoIPtr<IReader> m_pReader;
   byte * m_pImageData;
};

///////////////////////////////////////

cTargaReader::cTargaReader(IReader * pReader)
 : m_pColorMap(NULL),
   m_pImageData(NULL)
{
   memset(&m_header, 0, sizeof(m_header));
   memset(&m_footer, 0, sizeof(m_footer));
   memset(m_szId, 0, sizeof(m_szId));
   m_pReader = pReader;
   Assert(pReader != NULL);
   if (pReader != NULL)
      pReader->AddRef();
}

///////////////////////////////////////

cTargaReader::~cTargaReader()
{
   if (m_pColorMap != NULL)
   {
      free(m_pColorMap);
      m_pColorMap = NULL;
   }

   if (m_pImageData != NULL)
   {
      free(m_pImageData);
      m_pImageData = NULL;
   }
}

///////////////////////////////////////

inline const sTargaHeader & cTargaReader::GetHeader() const
{
   return m_header;
}

///////////////////////////////////////

inline byte * cTargaReader::GetImageData()
{
   return m_pImageData;
}

///////////////////////////////////////

bool cTargaReader::ReadHeader()
{
   if (AccessReader()->Read(&m_header) != S_OK)
   {
      return false;
   }

   if (m_header.IDLength > 0)
   {
      if (AccessReader()->Read(m_szId, m_header.IDLength) != S_OK)
      {
         return false;
      }
   }

   return true;
}

///////////////////////////////////////

bool cTargaReader::ReadFooter()
{
   if (AccessReader()->Read(&m_footer) == S_OK)
      return true;
   return false;
}

///////////////////////////////////////

bool cTargaReader::ReadColorMap()
{
   Assert(m_pColorMap == NULL);

   if (GetHeader().ColorMapType == 1)
   {
      uint colorMapEntryBytes = GetHeader().ColorMapEntrySize / 8;
      if (colorMapEntryBytes == 0)
      {
         return false;
      }

      m_pColorMap = (byte *)malloc(GetHeader().ColorMapLength * colorMapEntryBytes);
      if (m_pColorMap == NULL)
         return false;

      if (AccessReader()->Read(m_pColorMap, colorMapEntryBytes * GetHeader().ColorMapLength) != S_OK)
      {
         return false;
      }
   }

   return true;
}

///////////////////////////////////////

bool cTargaReader::ReadImageData()
{
   Assert(m_pImageData == NULL);

   uint bytesPerPixel = GetHeader().PixelDepth / 8;
   uint imageDataSize = bytesPerPixel * GetHeader().Width * GetHeader().Height;

   m_pImageData = (byte *)malloc(imageDataSize);
   if (m_pImageData == NULL)
      return false;

   bool bResult = false; // assume failure

   if (GetHeader().ImageType == kTGA_UncomprRGB ||
       GetHeader().ImageType == kTGA_UncomprBW)
   {
      if (GetHeader().ColorMapType == 0 &&
          AccessReader()->Read(m_pImageData, imageDataSize) == S_OK)
      {
         bResult = true;
      }
   }
   else if (GetHeader().ImageType == kTGA_UncomprColorMap)
   {
      if (GetHeader().ColorMapType == 1 &&
          AccessReader()->Read(m_pImageData, imageDataSize) == S_OK)
      {
         bResult = true;
      }
   }
   else if (GetHeader().ImageType == kTGA_RLEColorMap ||
            GetHeader().ImageType == kTGA_RLERGB)
   {
      uint8 bytesPerPixel = GetHeader().PixelDepth / 8;

      bool bReadError = false;

      byte * pPixelValue = (byte *)alloca(bytesPerPixel);

      byte * pDecoded = GetImageData();
      byte * pEnd = GetImageData() + imageDataSize;
      while (pDecoded < pEnd)
      {
         uint8 repCount;
         AccessReader()->Read(&repCount);

         if (repCount & 0x80) // is run-length packet?
         {
            repCount = (repCount & 0x7F) + 1;
            if (AccessReader()->Read(pPixelValue, bytesPerPixel) != S_OK)
            {
               DebugMsg("Error reading targa RLE packet\n");
               bReadError = true;
               break;
            }
            for (int i = 0; i < repCount; i++)
               memcpy(pDecoded + (i * bytesPerPixel), pPixelValue, bytesPerPixel);
         }
         else
         {
            repCount += 1;
            if (AccessReader()->Read(pDecoded, bytesPerPixel * repCount) != S_OK)
            {
               DebugMsg("Error reading targa raw packet\n");
               bReadError = true;
               break;
            }
         }

         pDecoded += bytesPerPixel * repCount;
      }

      if (!bReadError)
         bResult = true;
   }
   else if (GetHeader().ImageType == kTGA_ComprBW)
   {
      DebugMsg("Compressed black & white targas not supported\n");
   }
   else
   {
      DebugMsg1("Unknown targa image type %d\n", GetHeader().ImageType);
   }

   return bResult;
}

///////////////////////////////////////

IImage * cTargaReader::CreateImage()
{
   if (GetImageData() != NULL)
   {
      ePixelFormat pixelFormat = kPF_ERROR;

      // if 32- or 24-bit pixel format, swap the red and blue to save
      // having to use BGR formats
      if (GetHeader().PixelDepth == 32 ||
          GetHeader().PixelDepth == 24)
      {
#if NO_BGR_FORMATS
         uint8 bytesPerPixel = GetHeader().PixelDepth / 8;
         for (int i = 0; i < (GetHeader().Width * GetHeader().Height); i++)
         {
            byte * pRed = GetImageData() + (i * bytesPerPixel);
            byte * pBlue = pRed + 2;
            byte temp = *pRed;
            *pRed = *pBlue;
            *pBlue = temp;
         }
         pixelFormat = (GetHeader().PixelDepth == 24) ? kPF_RGB888 : kPF_RGBA8888;
#else
         pixelFormat = (GetHeader().PixelDepth == 24) ? kPF_BGR888 : kPF_BGRA8888;
#endif
      }
      else if (GetHeader().PixelDepth == 16)
      {
         if ((GetHeader().ImageDescriptor & 3) == 1)
            pixelFormat = kPF_RGB555;
         else
            pixelFormat = kPF_RGB565;
      }
      else if (GetHeader().PixelDepth == 8)
      {
         if (GetHeader().ColorMapType == 0)
         {
            pixelFormat = kPF_Grayscale;
         }
         else if (GetHeader().ColorMapType == 1)
         {
            if (GetHeader().ColorMapEntrySize == 24)
            {
               pixelFormat = kPF_RGB888;

               byte * pData = (byte *)malloc(3 * GetHeader().Width * GetHeader().Height);

               for (int i = 0; i < (GetHeader().Width * GetHeader().Height); i++)
               {
                  byte * pS = m_pColorMap + (GetImageData()[i] * GetHeader().ColorMapEntrySize / 8);
                  byte * pD = pData + (i * 3);
                  // the color map is stored as BGR so swap red and blue while we're at it
                  pD[0] = pS[2];
                  pD[1] = pS[1];
                  pD[2] = pS[0];
               }

               free(m_pImageData);
               m_pImageData = pData;
            }
         }
      }

      if (pixelFormat != kPF_ERROR)
      {
         cAutoIPtr<IImage> pImage;
         if (ImageCreate(GetHeader().Width, GetHeader().Height, pixelFormat, GetImageData(), &pImage) == S_OK)
         {
            return CTAddRef(pImage);
         }
      }
   }

   return NULL;
}


///////////////////////////////////////////////////////////////////////////////

void * TargaLoad(IReader * pReader)
{
   if (pReader != NULL)
   {
      cTargaReader targaReader(pReader);
      if (targaReader.ReadHeader()
         && targaReader.ReadFooter()
         && targaReader.ReadColorMap()
         && targaReader.ReadImageData())
      {
         return targaReader.CreateImage();
      }
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
