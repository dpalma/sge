/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TOOLPALETTE_H)
#define INCLUDED_TOOLPALETTE_H

#include <atlscrl.h>

#include <string>
#include <vector>
#include <map>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////////////////////////////////////////////////////

DECLARE_HANDLE(HTOOLGROUP);
DECLARE_HANDLE(HTOOLITEM);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolItem
//

class cToolItem
{
   cToolItem(const cToolItem &);
   const cToolItem & operator =(const cToolItem &);

public:
   cToolItem(const tChar * pszName, int iImage, void * pUserData);
   ~cToolItem();

   const tChar * GetName() const;
   int GetImageIndex() const;
   void * GetUserData() const;

private:
   std::string m_name;
   int m_iImage;
   void * m_pUserData;
};

////////////////////////////////////////

inline const tChar * cToolItem::GetName() const
{
   return m_name.c_str();
}

////////////////////////////////////////

inline int cToolItem::GetImageIndex() const
{
   return m_iImage;
}

////////////////////////////////////////

inline void * cToolItem::GetUserData() const
{
   return m_pUserData;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolGroup
//

class cToolGroup
{
   cToolGroup(const cToolGroup &);
   const cToolGroup & operator =(const cToolGroup &);

public:
   cToolGroup(const tChar * pszName, HIMAGELIST hImageList);
   ~cToolGroup();

   const tChar * GetName() const;
   HIMAGELIST GetImageList() const;
   uint GetToolCount() const;
   cToolItem * GetTool(uint index) const;

   HTOOLITEM AddTool(const tChar * pszTool, int iImage, void * pUserData);
   bool RemoveTool(HTOOLITEM hTool);
   HTOOLITEM FindTool(const tChar * pszTool);
   bool IsTool(HTOOLITEM hTool) const;
   void Clear();

private:
   std::string m_name;
   HIMAGELIST m_hImageList;

   typedef std::vector<cToolItem *> tTools;
   tTools m_tools;
};

////////////////////////////////////////

typedef std::vector<cToolGroup *> tGroups;

////////////////////////////////////////

inline const tChar * cToolGroup::GetName() const
{
   return m_name.c_str();
}

////////////////////////////////////////

inline HIMAGELIST cToolGroup::GetImageList() const
{
   return m_hImageList;
}

////////////////////////////////////////

inline uint cToolGroup::GetToolCount() const
{
   return m_tools.size();
}

////////////////////////////////////////

inline cToolItem * cToolGroup::GetTool(uint index) const
{
   if (index < m_tools.size())
   {
      return m_tools[index];
   }
   return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteRenderer
//
// Handles painting for cToolPalette. Called in a loop for each group to be
// rendered. Also caches size/position information for use in hit-testing.

class cToolPaletteRenderer
{
public:
   cToolPaletteRenderer();
   cToolPaletteRenderer(const cToolPaletteRenderer &);
   ~cToolPaletteRenderer();
   const cToolPaletteRenderer & operator =(const cToolPaletteRenderer &);

   bool Begin(HDC hDC, LPCRECT pRect, const POINT * pMousePos);
   bool End();

   HANDLE GetHitItem(const CPoint & point, RECT * pRect = NULL);
   bool GetItemRect(HANDLE hItem, RECT * pRect);

   void Render(tGroups::const_iterator from, tGroups::const_iterator to);
   void Render(const cToolGroup * pGroup);

   inline void FlushCachedRects() { m_cachedRects.clear(); }

   inline void operator ()(const cToolGroup * pGroup) { Render(pGroup); }

private:
   int RenderGroupHeading(const cToolGroup * pGroup);

   typedef std::map<HANDLE, CRect> tCachedRects;
   tCachedRects m_cachedRects;

   // These member variables valid only between Begin() and End() calls
   CDCHandle m_dc;
   CRect m_rect;
   CPoint m_mousePos;
   bool m_bHaveMousePos;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPalette
//

class cToolPalette : public CScrollWindowImpl<cToolPalette>
{
   typedef CScrollWindowImpl<cToolPalette> tToolPaletteBase;

public:
   cToolPalette();
   ~cToolPalette();

   DECLARE_WND_CLASS("ToolPalette")

   BEGIN_MSG_MAP_EX(cToolPalette)
      MSG_WM_CREATE(OnCreate)
      MSG_WM_DESTROY(OnDestroy)
      MSG_WM_SIZE(OnSize)
      MSG_WM_SETFONT(OnSetFont)
      MSG_WM_ERASEBKGND(OnEraseBkgnd);
      MSG_WM_PAINT(OnPaint)
      MSG_WM_MOUSELEAVE(OnMouseLeave)
      MSG_WM_MOUSEMOVE(OnMouseMove)
      MSG_WM_LBUTTONDOWN(OnLButtonDown)
      MSG_WM_LBUTTONUP(OnLButtonUp)
      CHAIN_MSG_MAP(tToolPaletteBase)
   END_MSG_MAP()

   LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
   void OnDestroy();
   void OnSize(UINT nType, CSize size);
   void OnSetFont(HFONT hFont, BOOL bRedraw);
   LRESULT OnEraseBkgnd(CDCHandle dc);
   void OnPaint(CDCHandle dc);
   void OnMouseLeave();
   void OnMouseMove(UINT flags, CPoint point);
   void OnLButtonDown(UINT flags, CPoint point);
   void OnLButtonUp(UINT flags, CPoint point);

   HTOOLGROUP AddGroup(const tChar * pszGroup, HIMAGELIST hImageList);
   bool RemoveGroup(HTOOLGROUP hGroup);
   HTOOLGROUP FindGroup(const tChar * pszGroup);
   bool IsGroup(HTOOLGROUP hGroup);
   void Clear();
   HTOOLITEM AddTool(HTOOLGROUP hGroup, const tChar * pszTool, int iImage, void * pUserData = NULL);
   bool RemoveTool(HTOOLITEM hTool);
   bool EnableTool(HTOOLITEM hTool);

private:
   void SetMouseOverItem(HANDLE hItem);

   tGroups m_groups;

   cToolPaletteRenderer m_renderer;

   CFont m_font;

   bool m_bTrackingMouseLeave;
   HANDLE m_hMouseOverItem;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_TOOLPALETTE_H)
