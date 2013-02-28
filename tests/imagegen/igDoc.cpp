/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "igDoc.h"

#include "tech/color.h"
#include "tech/filespec.h"
#include "tech/filepath.h"
#include "tech/globalobj.h"
#include "tech/point2.h"
#include "tech/point2.inl"
#include "tech/readwriteapi.h"
#include "tech/resourceapi.h"
#include "tech/techmath.h"


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenDoc
//

cImageGenDoc::cImageGenDoc()
: m_shape(AquaButton)
, m_bApplyGamma(false)
, m_gamma(1)
, m_defaultPixelFormat(kPF_BGR888)
, m_defaultImageWidth(256)
, m_defaultImageHeight(128)
{
}

cImageGenDoc::~cImageGenDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

void ImageSolidRect(IImage * pImage, uint x, uint y, uint w, uint h, const byte rgba[4])
{
   if (pImage == NULL)
   {
      return;
   }

   for (uint j = y; j < (y + h); ++j)
   {
      for (uint i = x; i < (x + w); ++i)
      {
         pImage->SetPixel(i, j, rgba);
      }
   }
}

void ImageSolidRect(IImage * pImage, const byte rgba[4])
{
   ImageSolidRect(pImage, 0, 0, pImage->GetWidth(), pImage->GetHeight(), rgba);
}

void ImageSolidCircle(IImage * pImage, uint x, uint y, uint radius, const byte rgba[4])
{
   if (pImage == NULL)
   {
      return;
   }

   uint imageWidth = pImage->GetWidth(), imageHeight = pImage->GetHeight();

   uint yStart = (y >= radius) ? y - radius : radius - y;
   uint yEnd = Min(y + radius, imageHeight);

   uint xStart = (x >= radius) ? x - radius : radius - x;
   uint xEnd = Min(x + radius, imageWidth);

   cPoint2<uint> center(x, y);
   int rSqr = radius * radius;

   for (uint j = yStart; j <= yEnd; ++j)
   {
      for (uint i = xStart; i <= xEnd; ++i)
      {
         int dSqr = DistanceSqr(cPoint2<uint>(i, j), center);
         if (dSqr < rSqr)
         {
            pImage->SetPixel(i, j, rgba);
         }
      }
   }
}

void ImageSolidCircle(IImage * pImage, const byte rgba[4])
{
   ImageSolidCircle(pImage, pImage->GetWidth() / 2, pImage->GetHeight() / 2, pImage->GetWidth() / 2, rgba);
}

enum eImageDrawFlags
{
   kIDF_Default         = 0,
   kIDF_AlphaBlend      = (1<<0),
};

enum eImageGradientDirection
{
   kIGD_LeftToRight,
   kIGD_TopToBottom,
};

void ImageGradientRoundRect(IImage * pImage, uint x, uint y, uint w, uint h, uint cornerRadius,
                            eImageGradientDirection dir, const cColor & startColor, const cColor & endColor,
                            uint flags)
{
   if (pImage == NULL)
   {
      return;
   }

   if (w == 0 || h == 0)
   {
      return;
   }

   uint cornerRadiusSqr = cornerRadius * cornerRadius;

   cPoint2<uint> topLeftCenter(x + cornerRadius, y + cornerRadius);
   cPoint2<uint> topRightCenter(x + w - cornerRadius - 1, y + cornerRadius);
   cPoint2<uint> bottomRightCenter(x + w - cornerRadius - 1, y + h - cornerRadius - 1);
   cPoint2<uint> bottomLeftCenter(x + cornerRadius, y + h - cornerRadius - 1);

   for (uint j = y; j < (y + h); ++j)
   {
      float fracY = static_cast<float>(j - y) / h;

      for (uint i = x; i < (x + w); ++i)
      {
         cPoint2<uint> ij(i, j);

         float fracX = static_cast<float>(i - x) / w;

         // frac is the fraction that should come from endColor
         // (e.g, fracY starts at 0 and goes to 1; same for fracX)
         float frac = 0;
         if (dir == kIGD_LeftToRight)
         {
            frac = fracX;
         }
         else if (dir == kIGD_TopToBottom)
         {
            frac = fracY;
         }

         cColor color(
            startColor.r * (1 - frac) + endColor.r * frac,
            startColor.g * (1 - frac) + endColor.g * frac,
            startColor.b * (1 - frac) + endColor.b * frac,
            startColor.a * (1 - frac) + endColor.a * frac);

         byte rgbaIJ[4];
         pImage->GetPixel(i, j, rgbaIJ);
         cColor pixelIJ(
            static_cast<float>(rgbaIJ[0]) / 255,
            static_cast<float>(rgbaIJ[1]) / 255,
            static_cast<float>(rgbaIJ[2]) / 255,
            static_cast<float>(rgbaIJ[3]) / 255);

         if ((flags & kIDF_AlphaBlend) == kIDF_AlphaBlend)
         {
            color.r = (color.r * color.a) + (pixelIJ.r * (1 - color.a));
            color.g = (color.g * color.a) + (pixelIJ.g * (1 - color.a));
            color.b = (color.b * color.a) + (pixelIJ.b * (1 - color.a));
            color.a = (color.a * color.a) + (pixelIJ.a * (1 - color.a));
         }

         if (i < topLeftCenter.x && j < topLeftCenter.y)
         {
            uint dSqr = DistanceSqr(ij, topLeftCenter);
            if (dSqr < cornerRadiusSqr || cornerRadiusSqr == 0)
            {
               pImage->SetPixel(i, j, cRGBA(color.GetPointer()));
            }
         }
         else if (i >= topRightCenter.x && j < topRightCenter.y)
         {
            uint dSqr = DistanceSqr(ij, topRightCenter);
            if (dSqr < cornerRadiusSqr || cornerRadiusSqr == 0)
            {
               pImage->SetPixel(i, j, cRGBA(color.GetPointer()));
            }
         }
         else if (i >= bottomRightCenter.x && j >= bottomRightCenter.y)
         {
            uint dSqr = DistanceSqr(ij, bottomRightCenter);
            if (dSqr < cornerRadiusSqr || cornerRadiusSqr == 0)
            {
               pImage->SetPixel(i, j, cRGBA(color.GetPointer()));
            }
         }
         else if (i < bottomLeftCenter.x && j >= bottomLeftCenter.y)
         {
            uint dSqr = DistanceSqr(ij, bottomLeftCenter);
            if (dSqr < cornerRadiusSqr || cornerRadiusSqr == 0)
            {
               pImage->SetPixel(i, j, cRGBA(color.GetPointer()));
            }
         }
         else
         {
            pImage->SetPixel(i, j, cRGBA(color.GetPointer()));
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////

void cImageGenDoc::SetImage(IImage * pImage)
{
   SafeRelease(m_pImage);
   m_pImage = CTAddRef(pImage);
   Rasterize();
}

void cImageGenDoc::SetShape(eShape shape)
{
   if (m_shape != shape)
   {
      m_shape = shape;
      Rasterize();
   }
}

void cImageGenDoc::SetGamma(float gamma)
{
   if (m_gamma != gamma)
   {
      m_gamma = gamma;
      if (IsGammaEnabled())
         Rasterize();
   }
}

void cImageGenDoc::EnableGamma(bool bEnable)
{
   if (m_bApplyGamma != bEnable)
   {
      m_bApplyGamma = bEnable;
      Rasterize();
   }
}

void cImageGenDoc::Rasterize()
{
   if (!m_pImage)
   {
      return;
   }

   if (m_shape == Circle)
   {
      ImageSolidRect(m_pImage, cRGBA(1,1,1));

      uint imageWidth = m_pImage->GetWidth(), imageHeight = m_pImage->GetHeight();
      uint radius = Min(imageWidth, imageHeight) / 2;
      ImageGradientRoundRect(m_pImage, 0, 0, radius * 2, radius * 2, radius,
                            kIGD_LeftToRight, cColor(0,1,0), cColor(0,1,0),
                            kIDF_Default);
   }
   else if (m_shape == Rectangle)
   {
      ImageGradientRoundRect(m_pImage, 0, 0, m_pImage->GetWidth(), m_pImage->GetHeight(),
         0, kIGD_LeftToRight, cColor(0,0,1), cColor(0,0,1), kIDF_Default);
   }
   else if (m_shape == RoundRect)
   {
      ImageSolidRect(m_pImage, cRGBA(1,1,1));
      ImageGradientRoundRect(m_pImage, 0, 0, m_pImage->GetWidth(), m_pImage->GetHeight(),
         Min(m_pImage->GetWidth(), m_pImage->GetHeight()) / 8,
         kIGD_LeftToRight, cColor(1,0,0), cColor(0,0,1), kIDF_Default);
   }
   else if (m_shape == AquaButton)
   {
      ImageSolidRect(m_pImage, cRGBA(1,1,1));

      int cornerRadius = Min(m_pImage->GetWidth(), m_pImage->GetHeight()) / 8;

      ImageGradientRoundRect(m_pImage, 0, 0, m_pImage->GetWidth(), m_pImage->GetHeight(),
         cornerRadius, kIGD_TopToBottom, cColor(1,0,0,1), cColor(1,1,1,1), kIDF_Default);

      int hlInset = Min(m_pImage->GetWidth(), m_pImage->GetHeight()) / 16;
      hlInset = 0;

      int hlHeight = 2 * cornerRadius;

      ImageGradientRoundRect(m_pImage, hlInset, hlInset, m_pImage->GetWidth() - (2 * hlInset), hlHeight,
         cornerRadius, kIGD_TopToBottom, cColor(1,1,1,.75f), cColor(1,1,1,0), kIDF_AlphaBlend);
   }
   else if (m_shape == Static)
   {
      uint imageWidth = m_pImage->GetWidth(), imageHeight = m_pImage->GetHeight();
      uint x = 0, y = 0, w = imageWidth, h = imageHeight;

      cRand r(GetTickCount());

      static const uint kMaxSeg = 3;

      byte gray = static_cast<byte>(r.Next() % 256);
      uint index = 0, segEnd = index + (r.Next() % kMaxSeg) + 1;

      for (uint j = y; j < (y + h); ++j)
      {
         for (uint i = x; i < (x + w); ++i, ++index)
         {
            if (index >= segEnd)
            {
               gray = static_cast<byte>(r.Next() % 256);
               segEnd = index + (r.Next() % kMaxSeg) + 1;
            }
            m_pImage->SetPixel(i, j, cRGBA(gray, gray, gray));
         }
      }
   }

   if (m_bApplyGamma)
   {
      ImageApplyGamma(m_pImage, 0, 0, m_pImage->GetWidth(), m_pImage->GetHeight(), m_gamma);
   }
}

IImage * cImageGenDoc::AccessImage()
{
   return m_pImage;
}

const IImage * cImageGenDoc::AccessImage() const
{
   return m_pImage;
}

bool cImageGenDoc::NewDocument()
{
   Assert(!m_pImage);
   SafeRelease(m_pImage);

   if (ImageCreate(m_defaultImageWidth, m_defaultImageHeight, m_defaultPixelFormat, NULL, &m_pImage) == S_OK)
   {
      Rasterize();
      return true;
   }

	return false;
}

void cImageGenDoc::DeleteContents()
{
   SafeRelease(m_pImage);
}

bool cImageGenDoc::OpenDocument(LPCTSTR lpszPathName)
{
   cFileSpec file(lpszPathName);

   cFilePath dir;
   if (file.GetPath(&dir))
   {
      UseGlobal(ResourceManager);
      if (pResourceManager->AddDirectory(dir.CStr()) == S_OK)
      {
         IImage * pImage = NULL;
         if (pResourceManager->Load(file.GetFileName(), kRT_Image, NULL, (void**)&pImage) == S_OK)
         {
            SafeRelease(m_pImage);
            // Resource manager doesn't AddRef and neither does cAutoIPtr when
            // assigning a raw interface pointer, so do an AddRef here.
            m_pImage = CTAddRef(pImage);
            m_fileName = file;
            m_shape = None;
            return true;
         }
      }
   }

   return false;
}

bool cImageGenDoc::SaveDocument(LPCTSTR lpszPathName)
{
   if (!!m_pImage)
   {
      cFileSpec file(lpszPathName);
      cAutoIPtr<IWriter> pWriter;
      if (FileWriterCreate(file, kFileModeBinary, &pWriter) == S_OK)
      {
         if (BmpWrite(m_pImage, pWriter) == S_OK)
         {
            m_fileName = file;
            return true;
         }
      }
   }

   return false;
}
