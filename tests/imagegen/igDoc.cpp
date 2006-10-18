/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "igDoc.h"

#include "ImageAttributesDlg.h"
#include "ImageGammaDlg.h"

#include "color.h"
#include "filespec.h"
#include "filepath.h"
#include "readwriteapi.h"
#include "techmath.h"
#include "vec2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenDoc
//

IMPLEMENT_DYNCREATE(cImageGenDoc, CDocument)

BEGIN_MESSAGE_MAP(cImageGenDoc, CDocument)
	//{{AFX_MSG_MAP(cImageGenDoc)
	//}}AFX_MSG_MAP
   ON_COMMAND(ID_IMAGE_ATTRIBUTES, &cImageGenDoc::OnImageAttributes)
   ON_COMMAND(ID_IMAGE_CIRCLE, &cImageGenDoc::OnImageCircle)
   ON_UPDATE_COMMAND_UI(ID_IMAGE_CIRCLE, &cImageGenDoc::OnUpdateImageCircle)
   ON_COMMAND(ID_IMAGE_RECTANGLE, &cImageGenDoc::OnImageRectangle)
   ON_UPDATE_COMMAND_UI(ID_IMAGE_RECTANGLE, &cImageGenDoc::OnUpdateImageRectangle)
   ON_COMMAND(ID_IMAGE_ROUNDRECT, &cImageGenDoc::OnImageRoundrect)
   ON_UPDATE_COMMAND_UI(ID_IMAGE_ROUNDRECT, &cImageGenDoc::OnUpdateImageRoundrect)
   ON_COMMAND(ID_IMAGE_AQUABUTTON, &cImageGenDoc::OnImageAquabutton)
   ON_UPDATE_COMMAND_UI(ID_IMAGE_AQUABUTTON, &cImageGenDoc::OnUpdateImageAquabutton)
   ON_COMMAND(ID_IMAGE_GAMMA, &cImageGenDoc::OnImageGamma)
   ON_COMMAND(ID_IMAGE_STATIC, &cImageGenDoc::OnImageStatic)
   ON_UPDATE_COMMAND_UI(ID_IMAGE_STATIC, &cImageGenDoc::OnUpdateImageStatic)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc construction/destruction

cImageGenDoc::cImageGenDoc()
: m_shape(kAquaButton)
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

   cVec2<uint> center(x, y);
   int rSqr = radius * radius;

   for (uint j = yStart; j <= yEnd; ++j)
   {
      for (uint i = xStart; i <= xEnd; ++i)
      {
         int dSqr = Vec2DistanceSqr(cVec2<uint>(i, j), center);
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

   cVec2<uint> topLeftCenter(x + cornerRadius, y + cornerRadius);
   cVec2<uint> topRightCenter(x + w - cornerRadius - 1, y + cornerRadius);
   cVec2<uint> bottomRightCenter(x + w - cornerRadius - 1, y + h - cornerRadius - 1);
   cVec2<uint> bottomLeftCenter(x + cornerRadius, y + h - cornerRadius - 1);

   for (uint j = y; j < (y + h); ++j)
   {
      float fracY = static_cast<float>(j - y) / h;

      for (uint i = x; i < (x + w); ++i)
      {
         cVec2<uint> ij(i, j);

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
            uint dSqr = Vec2DistanceSqr(ij, topLeftCenter);
            if (dSqr < cornerRadiusSqr || cornerRadiusSqr == 0)
            {
               pImage->SetPixel(i, j, cRGBA(color.GetPointer()));
            }
         }
         else if (i >= topRightCenter.x && j < topRightCenter.y)
         {
            uint dSqr = Vec2DistanceSqr(ij, topRightCenter);
            if (dSqr < cornerRadiusSqr || cornerRadiusSqr == 0)
            {
               pImage->SetPixel(i, j, cRGBA(color.GetPointer()));
            }
         }
         else if (i >= bottomRightCenter.x && j >= bottomRightCenter.y)
         {
            uint dSqr = Vec2DistanceSqr(ij, bottomRightCenter);
            if (dSqr < cornerRadiusSqr || cornerRadiusSqr == 0)
            {
               pImage->SetPixel(i, j, cRGBA(color.GetPointer()));
            }
         }
         else if (i < bottomLeftCenter.x && j >= bottomLeftCenter.y)
         {
            uint dSqr = Vec2DistanceSqr(ij, bottomLeftCenter);
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

void cImageGenDoc::Rasterize()
{
   if (!m_pImage)
   {
      return;
   }

   if (m_shape == kCircle)
   {
      ImageSolidRect(m_pImage, cRGBA(1,1,1));

      uint imageWidth = m_pImage->GetWidth(), imageHeight = m_pImage->GetHeight();
      uint radius = Min(imageWidth, imageHeight) / 2;
      ImageGradientRoundRect(m_pImage, 0, 0, radius * 2, radius * 2, radius,
                            kIGD_LeftToRight, cColor(0,1,0), cColor(0,1,0),
                            kIDF_Default);
   }
   else if (m_shape == kRectangle)
   {
      ImageGradientRoundRect(m_pImage, 0, 0, m_pImage->GetWidth(), m_pImage->GetHeight(),
         0, kIGD_LeftToRight, cColor(0,0,1), cColor(0,0,1), kIDF_Default);
   }
   else if (m_shape == kRoundRect)
   {
      ImageSolidRect(m_pImage, cRGBA(1,1,1));
      ImageGradientRoundRect(m_pImage, 0, 0, m_pImage->GetWidth(), m_pImage->GetHeight(),
         Min(m_pImage->GetWidth(), m_pImage->GetHeight()) / 8,
         kIGD_LeftToRight, cColor(1,0,0), cColor(0,0,1), kIDF_Default);
   }
   else if (m_shape == kAquaButton)
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
   else if (m_shape == kStatic)
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

   UpdateAllViews(NULL);
}

/////////////////////////////////////////////////////////////////////////////

BOOL cImageGenDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Add reinitialization code here (SDI documents will reuse this document)

   BOOL bResult = FALSE;

   Assert(!m_pImage);

   if (ImageCreate(m_defaultImageWidth, m_defaultImageHeight, m_defaultPixelFormat, NULL, &m_pImage) == S_OK)
   {
      bResult = TRUE;

      Rasterize();
   }

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc diagnostics

#ifdef _DEBUG
void cImageGenDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void cImageGenDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc operations

IImage * cImageGenDoc::AccessImage()
{
   return m_pImage;
}

const IImage * cImageGenDoc::AccessImage() const
{
   return m_pImage;
}

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc commands

void cImageGenDoc::DeleteContents()
{
   SafeRelease(m_pImage);

   CDocument::DeleteContents();
}

BOOL cImageGenDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
   if (!!m_pImage)
   {
      cAutoIPtr<IWriter> pWriter;
      if (FileWriterCreate(cFileSpec(lpszPathName), kFileModeBinary, &pWriter) == S_OK)
      {
         if (BmpWrite(m_pImage, pWriter) == S_OK)
         {
            return TRUE;
         }
      }
   }

   return CDocument::OnSaveDocument(lpszPathName);
}

void cImageGenDoc::OnImageAttributes()
{
   if (!m_pImage)
   {
      return;
   }

   ePixelFormat pixelFormat = m_pImage->GetPixelFormat();
   uint imageWidth = m_pImage->GetWidth(), imageHeight = m_pImage->GetHeight();

   cImageAttributesDlg dlg;
   dlg.m_pixelFormat = pixelFormat;
   dlg.m_width = imageWidth;
   dlg.m_height = imageHeight;
   if (dlg.DoModal() == IDOK)
   {
      pixelFormat = (ePixelFormat)dlg.m_pixelFormat;
      imageWidth = dlg.m_width;
      imageHeight = dlg.m_height;

      SafeRelease(m_pImage);
      if (ImageCreate(imageWidth, imageHeight, pixelFormat, NULL, &m_pImage) == S_OK)
      {
         Rasterize();
      }
   }
}

void cImageGenDoc::OnImageCircle()
{
   m_shape = kCircle;
   Rasterize();
}

void cImageGenDoc::OnUpdateImageCircle(CCmdUI *pCmdUI)
{
   pCmdUI->SetRadio(m_shape == kCircle);
}

void cImageGenDoc::OnImageRectangle()
{
   m_shape = kRectangle;
   Rasterize();
}

void cImageGenDoc::OnUpdateImageRectangle(CCmdUI *pCmdUI)
{
   pCmdUI->SetRadio(m_shape == kRectangle);
}

void cImageGenDoc::OnImageRoundrect()
{
   m_shape = kRoundRect;
   Rasterize();
}

void cImageGenDoc::OnUpdateImageRoundrect(CCmdUI *pCmdUI)
{
   pCmdUI->SetRadio(m_shape == kRoundRect);
}

void cImageGenDoc::OnImageAquabutton()
{
   m_shape = kAquaButton;
   Rasterize();
}

void cImageGenDoc::OnUpdateImageAquabutton(CCmdUI *pCmdUI)
{
   pCmdUI->SetRadio(m_shape == kAquaButton);
}

void cImageGenDoc::OnImageStatic()
{
   m_shape = kStatic;
   Rasterize();
}

void cImageGenDoc::OnUpdateImageStatic(CCmdUI *pCmdUI)
{
   pCmdUI->SetRadio(m_shape == kStatic);
}

void cImageGenDoc::OnImageGamma()
{
   cImageGammaDlg dlg;
   dlg.m_bApplyGamma = m_bApplyGamma;
   dlg.m_gamma = m_gamma;
   if (dlg.DoModal() == IDOK)
   {
      m_bApplyGamma = dlg.m_bApplyGamma ? true : false;
      m_gamma = dlg.m_gamma;

      Rasterize();
   }
}
