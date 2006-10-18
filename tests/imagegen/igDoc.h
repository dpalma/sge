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
   void Rasterize();

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
   afx_msg void OnImageCircle();
   afx_msg void OnUpdateImageCircle(CCmdUI *pCmdUI);
   afx_msg void OnImageRectangle();
   afx_msg void OnUpdateImageRectangle(CCmdUI *pCmdUI);
   afx_msg void OnImageRoundrect();
   afx_msg void OnUpdateImageRoundrect(CCmdUI *pCmdUI);
   afx_msg void OnImageAquabutton();
   afx_msg void OnUpdateImageAquabutton(CCmdUI *pCmdUI);
   afx_msg void OnImageStatic();
   afx_msg void OnUpdateImageStatic(CCmdUI *pCmdUI);
   afx_msg void OnImageGamma();
	DECLARE_MESSAGE_MAP()

private:
   enum eShape
   {
      kCircle, kRectangle, kRoundRect, kAquaButton, kStatic
   };

   eShape m_shape;

   bool m_bApplyGamma;
   float m_gamma;

   ePixelFormat m_defaultPixelFormat;
   uint m_defaultImageWidth, m_defaultImageHeight;

   cAutoIPtr<IImage> m_pImage;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_IGDOC_H)
