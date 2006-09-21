/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "igDoc.h"

#include "ImageAttributesDlg.h"

#include "color.h"
#include "filespec.h"
#include "filepath.h"
#include "readwriteapi.h"
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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc construction/destruction

cImageGenDoc::cImageGenDoc()
: m_shape(kRectangle)
, m_defaultPixelFormat(kPF_BGR888)
, m_defaultImageWidth(256)
, m_defaultImageHeight(256)
{
}

cImageGenDoc::~cImageGenDoc()
{
}

/////////////////////////////////////////////////////////////////////////////

void ImageSolidRect(IImage * pImage, uint x, uint y, uint w, uint h, const cColor & color)
{
   if (pImage == NULL)
   {
      return;
   }

   for (uint j = y; j < (y + h); ++j)
   {
      for (uint i = x; i < (x + w); ++i)
      {
         pImage->SetPixel(i, j, color);
      }
   }
}

void ImageSolidRect(IImage * pImage, const cColor & color)
{
   ImageSolidRect(pImage, 0, 0, pImage->GetWidth(), pImage->GetHeight(), color);
}

void ImageSolidCircle(IImage * pImage, uint x, uint y, uint radius, const cColor & color)
{
   if (pImage == NULL)
   {
      return;
   }

   uint imageWidth = pImage->GetWidth(), imageHeight = pImage->GetHeight();

   int yStart = (y >= radius) ? y - radius : radius - y;
   int yEnd = Min(y + radius, imageHeight);

   int xStart = (x >= radius) ? x - radius : radius - x;
   int xEnd = Min(x + radius, imageWidth);

   cVec2<int> center(x, y);
   int rSqr = radius * radius;

   for (int j = yStart; j <= yEnd; ++j)
   {
      for (int i = xStart; i <= xEnd; ++i)
      {
         int dSqr = Vec2DistanceSqr(cVec2<int>(i, j), center);
         if (dSqr < rSqr)
         {
            pImage->SetPixel(i, j, color);
         }
      }
   }
}

void ImageSolidCircle(IImage * pImage, const cColor & color)
{
   ImageSolidCircle(pImage, pImage->GetWidth() / 2, pImage->GetHeight() / 2, pImage->GetWidth() / 2, color);
}

void ImageApplyGamma(IImage * pImage, uint x, uint y, uint w, uint h, float gamma)
{
   if (pImage == NULL)
   {
      return;
   }

   if (gamma < 0.2f || gamma > 5.0f)
   {
      WarnMsg1("Unusual gamma, %f\n", gamma);
   }

   float oneOverGamma = 1.0f / gamma;

   float red[256], green[256], blue[256];
   for (int i = 0; i < 256; ++i)
   {
      float value = (255 * pow(static_cast<float>(i) / 255, oneOverGamma)) + 0.5f;
      value = Min(value, 255);
      red[i] = value;
      green[i] = value;
      blue[i] = value;
   }
}

template <>
inline uint Vec2Distance(const cVec2<uint> & v1, const cVec2<uint> & v2)
{
   return static_cast<uint>(FloatToInt(sqrt(static_cast<float>(Vec2DistanceSqr(v1, v2)))));
}

enum eCorner
{
   kTopLeft,
   kTopRight,
   kBottomRight,
   kBottomLeft,
};

void ImageGradientRoundRect(IImage * pImage, uint x, uint y, uint w, uint h, uint cornerRadius, uint flags, const cColor colors[4])
{
   if (pImage == NULL)
   {
      return;
   }

   if (w == 0 || h == 0)
   {
      return;
   }

   int cornerRadiusSqr = cornerRadius * cornerRadius;

   cVec2<uint> topLeftCenter(x + cornerRadius, y + cornerRadius);
   cVec2<uint> topRightCenter(x + w - cornerRadius - 1, y + cornerRadius);
   cVec2<uint> bottomRightCenter(x + w - cornerRadius - 1, y + h - cornerRadius - 1);
   cVec2<uint> bottomLeftCenter(x + cornerRadius, y + h - cornerRadius - 1);

   for (uint j = y; j < (y + h); ++j)
   {
      for (uint i = x; i < (x + w); ++i)
      {
         cVec2<uint> ij(i, j);

         uint topLeftDist = Vec2Distance(ij, cVec2<uint>(x,y));
         uint topRightDist = Vec2Distance(ij, cVec2<uint>(x+w,y));
         uint bottomRightDist = Vec2Distance(ij, cVec2<uint>(x+w,y+h));
         uint bottomLeftDist = Vec2Distance(ij, cVec2<uint>(x,y+h));

         uint sumDist = topLeftDist + topRightDist + bottomRightDist + bottomLeftDist;

         float topLeftFrac = 1.0f - (static_cast<float>(topLeftDist) / sumDist);
         float topRightFrac = 1.0f - (static_cast<float>(topRightDist) / sumDist);
         float bottomRightFrac = 1.0f - (static_cast<float>(bottomRightDist) / sumDist);
         float bottomLeftFrac = 1.0f - (static_cast<float>(bottomLeftDist) / sumDist);

         cColor color(
            colors[kTopLeft].r * topLeftFrac + colors[kTopRight].r * topRightFrac + colors[kBottomRight].r * bottomRightFrac + colors[kBottomLeft].r * bottomLeftFrac,
            colors[kTopLeft].g * topLeftFrac + colors[kTopRight].g * topRightFrac + colors[kBottomRight].g * bottomRightFrac + colors[kBottomLeft].g * bottomLeftFrac,
            colors[kTopLeft].b * topLeftFrac + colors[kTopRight].b * topRightFrac + colors[kBottomRight].b * bottomRightFrac + colors[kBottomLeft].b * bottomLeftFrac,
            colors[kTopLeft].a * topLeftFrac + colors[kTopRight].a * topRightFrac + colors[kBottomRight].a * bottomRightFrac + colors[kBottomLeft].a * bottomLeftFrac);

         cColor pixelIJ;
         pImage->GetPixel(i, j, &pixelIJ);

         if (i < topLeftCenter.x && j < topLeftCenter.y)
         {
            int dSqr = Vec2DistanceSqr(ij, topLeftCenter);
            if (dSqr < cornerRadiusSqr)
            {
               pImage->SetPixel(i, j, color);
            }
         }
         else if (i >= topRightCenter.x && j < topRightCenter.y)
         {
            int dSqr = Vec2DistanceSqr(ij, topRightCenter);
            if (dSqr < cornerRadiusSqr)
            {
               pImage->SetPixel(i, j, color);
            }
         }
         else if (i >= bottomRightCenter.x && j >= bottomRightCenter.y)
         {
            int dSqr = Vec2DistanceSqr(ij, bottomRightCenter);
            if (dSqr < cornerRadiusSqr)
            {
               pImage->SetPixel(i, j, color);
            }
         }
         else if (i < bottomLeftCenter.x && j >= bottomLeftCenter.y)
         {
            int dSqr = Vec2DistanceSqr(ij, bottomLeftCenter);
            if (dSqr < cornerRadiusSqr)
            {
               pImage->SetPixel(i, j, color);
            }
         }
         else
         {
            pImage->SetPixel(i, j, color);
         }
      }
   }
}

void ImageSolidRoundRect(IImage * pImage, uint x, uint y, uint w, uint h, uint cornerRadius, const cColor & color)
{
   cColor colors[4] = { color, color, color, color };
   ImageGradientRoundRect(pImage, x, y, w, h, cornerRadius, 0, colors);
}

void ImageSolidRoundRect(IImage * pImage, const cColor & color)
{
   uint cr = Min(pImage->GetWidth(), pImage->GetHeight()) / 4;
   ImageSolidRoundRect(pImage, 0, 0, pImage->GetWidth(), pImage->GetHeight(), cr, color);
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
      ImageSolidRect(m_pImage, cColor(1,1,1));
      ImageSolidCircle(m_pImage, cColor(0,1,0));
   }
   else if (m_shape == kRectangle)
   {
      ImageSolidRect(m_pImage, cColor(0,0,1));
   }
   else if (m_shape == kRoundRect)
   {
      ImageSolidRect(m_pImage, cColor(1,1,1));
      cColor colors[4] = { cColor(1,0,0), cColor(0,1,0), cColor(0,0,1), cColor(1,1,1) };
      ImageGradientRoundRect(m_pImage, 0, 0, m_pImage->GetWidth(), m_pImage->GetHeight(),
         Min(m_pImage->GetWidth(), m_pImage->GetHeight()) / 8, 0, colors);
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
      if (FileWriterCreate(cFileSpec(lpszPathName), &pWriter) == S_OK)
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
