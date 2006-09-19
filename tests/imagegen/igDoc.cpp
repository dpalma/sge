/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "igDoc.h"

#include "ImageAttributesDlg.h"

#include "color.h"
#include "filespec.h"
#include "filepath.h"
#include "readwriteapi.h"

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

void cImageGenDoc::Rasterize()
{
   if (!m_pImage)
   {
      return;
   }

   uint imageHeight = m_pImage->GetHeight(), imageWidth = m_pImage->GetWidth();

   if (m_shape == kCircle)
   {
      // TODO
   }
   else if (m_shape == kRectangle)
   {
      for (uint j = 0; j < imageHeight; ++j)
      {
         for (uint i = 0; i < imageWidth; ++i)
         {
            m_pImage->SetPixel(i, j, cColor(0,0,1));
         }
      }
   }
   else if (m_shape == kRoundRect)
   {
      // TODO
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
