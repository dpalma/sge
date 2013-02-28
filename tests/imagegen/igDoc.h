/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_IGDOC_H)
#define INCLUDED_IGDOC_H

#include "tech/imageapi.h"
#include "tech/filespec.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenDoc
//

class cImageGenDoc
{
public:
	cImageGenDoc();
	~cImageGenDoc();

   void SetImage(IImage * pImage);
   IImage * AccessImage();
   const IImage * AccessImage() const;

   enum eShape
   {
      None, Circle, Rectangle, RoundRect, AquaButton, Static
   };
   void SetShape(eShape shape);
   eShape GetShape() const { return m_shape; }
   void SetGamma(float gamma);
   float GetGamma() const { return m_gamma; }
   void EnableGamma(bool bEnable);
   bool IsGammaEnabled() const { return m_bApplyGamma; }
   void Rasterize();

   const cFileSpec & GetFileName() const { return m_fileName; }

	bool NewDocument();
   void DeleteContents();
   bool OpenDocument(LPCTSTR lpszPathName);
   bool SaveDocument(LPCTSTR lpszPathName);

private:
   cFileSpec m_fileName;

   eShape m_shape;

   bool m_bApplyGamma;
   float m_gamma;

   ePixelFormat m_defaultPixelFormat;
   uint m_defaultImageWidth, m_defaultImageHeight;

   cAutoIPtr<IImage> m_pImage;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_IGDOC_H)
