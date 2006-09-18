/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_IGDOC_H)
#define INCLUDED_IGDOC_H

#include "comtools.h"
#include "imageapi.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenDoc
//

class cImageGenDoc : public CDocument
{
protected: // create from serialization only
	cImageGenDoc();
	DECLARE_DYNCREATE(cImageGenDoc)

// Attributes
public:
   IImage * AccessImage();
   const IImage * AccessImage() const;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cImageGenDoc)
	public:
	virtual BOOL OnNewDocument();
//	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL
   virtual void DeleteContents();
   virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);

// Implementation
public:
	virtual ~cImageGenDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(cImageGenDoc)
	//}}AFX_MSG
   afx_msg void OnImageAttributes();
	DECLARE_MESSAGE_MAP()

private:
   ePixelFormat m_pixelFormat;
   uint m_imageWidth, m_imageHeight;
   cAutoIPtr<IImage> m_pImage;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_IGDOC_H)
