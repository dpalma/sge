/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TOOLPALETTE_H)
#define INCLUDED_TOOLPALETTE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPalette
//

DECLARE_HANDLE(HTOOLGROUP);
DECLARE_HANDLE(HTOOLITEM);

const int kMaxToolString = 50;

struct sToolGroup
{
   char szTitle[kMaxToolString];
};

struct sToolItem
{
   char szTitle[kMaxToolString];
};

class cToolPalette : public CWindowImpl<cToolPalette>
{
   typedef CWindowImpl<cToolPalette> tToolPaletteBase;

public:
   cToolPalette();
   ~cToolPalette();

   DECLARE_WND_CLASS("ToolPalette")

   BEGIN_MSG_MAP_EX(cToolPalette)
      MSG_WM_CREATE(OnCreate)
      MSG_WM_DESTROY(OnDestroy)
	   MSG_WM_SIZE(OnSize)
      MSG_WM_PAINT(OnPaint)
   END_MSG_MAP()

   LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
   void OnDestroy();
   void OnSize(UINT nType, CSize size);
   void OnPaint(CDCHandle dc);

   HTOOLGROUP AddGroup(const sToolGroup * pGroup);
   HTOOLITEM AddItem(HTOOLGROUP hGroup, const sToolItem * pItem);

private:
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_TOOLPALETTE_H)
