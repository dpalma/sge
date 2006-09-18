/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "igDoc.h"

#include "ImageAttributesDlg.h"

#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"

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
   ON_COMMAND(ID_IMAGE_ATTRIBUTES, OnImageAttributes)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cImageGenDoc construction/destruction

cImageGenDoc::cImageGenDoc()
: m_pixelFormat(kPF_RGBA8888)
, m_imageWidth(256)
, m_imageHeight(256)
{
}

cImageGenDoc::~cImageGenDoc()
{
}

BOOL cImageGenDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Add reinitialization code here (SDI documents will reuse this document)

   BOOL bResult = FALSE;

   Assert(!m_pImage);

   if (ImageCreate(m_imageWidth, m_imageHeight, m_pixelFormat, NULL, &m_pImage) == S_OK)
   {
      bResult = TRUE;
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
   cImageAttributesDlg dlg;
   dlg.m_pixelFormat = m_pixelFormat;
   dlg.m_width = m_imageWidth;
   dlg.m_height = m_imageHeight;
   if (dlg.DoModal() == IDOK)
   {
      m_pixelFormat = (ePixelFormat)dlg.m_pixelFormat;
      m_imageWidth = dlg.m_width;
      m_imageHeight = dlg.m_height;
   }
}
