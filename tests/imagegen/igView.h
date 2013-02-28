/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_IGVIEW_H)
#define INCLUDED_IGVIEW_H

#include <atlscrl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cImageGenDoc;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenView
//

class cImageGenView : public WTL::CScrollWindowImpl<cImageGenView>
{
public:
	cImageGenView(const cImageGenDoc * pDoc);
	~cImageGenView();

   void SetDocument(const cImageGenDoc * pDoc) { m_pDoc = pDoc; }
   const cImageGenDoc * GetDocument() const { return m_pDoc; }

   void Update();

	BEGIN_MSG_MAP(cImageGenView)
		CHAIN_MSG_MAP(WTL::CScrollWindowImpl<cImageGenView>);
	END_MSG_MAP()

   void DoPaint(WTL::CDCHandle dc);

private:
   const cImageGenDoc * m_pDoc;
   WTL::CBitmap m_bitmap;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_IGVIEW_H)
