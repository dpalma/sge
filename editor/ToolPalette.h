/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TOOLPALETTE_H)
#define INCLUDED_TOOLPALETTE_H

#include "TrackMouseEvent.h"

#include <atlscrl.h>

#include <string>
#include <vector>
#include <map>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cToolItem;
class cToolGroup;

///////////////////////////////////////////////////////////////////////////////

DECLARE_HANDLE(HTOOLGROUP);
DECLARE_HANDLE(HTOOLITEM);

///////////////////////////////////////////////////////////////////////////////

const int kMaxName = 50;

enum eToolPaletteToolState
{
   kTPTS_None = 0,
   kTPTS_Disabled = (1<<0),
   kTPTS_Checked = (1<<1),
};

struct sToolPaletteItem
{
   char szName[kMaxName];
   uint state;
   int iImage;
   void * pUserData;
};

enum eToolPaletteStyle
{
   kTPS_None = 0,
   kTPS_ExclusiveCheck = (1<<0), // only one item checked at a time
};

///////////////////////////////////////////////////////////////////////////////

enum eToolPaletteNotifyCodes
{
   kTPN_ItemClick       = 0xD100,
   kTPN_ItemDestroy     = 0xD101,
   kTPN_ItemCheck       = 0xD102,
   kTPN_ItemUncheck     = 0xD103,
};

///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sNMToolPaletteItem
//
// Info struct for most item-based notification messages

struct sNMToolPaletteItem
{
   NMHDR hdr;
   POINT pt;
   HTOOLITEM hTool;
   void * pUserData;
};

typedef sNMToolPaletteItem sNMToolPaletteItemClick;
typedef sNMToolPaletteItem sNMToolPaletteItemDestroy;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolItem
//

class cToolItem
{
   cToolItem(const cToolItem &);
   const cToolItem & operator =(const cToolItem &);

public:
   cToolItem(cToolGroup * pGroup, const tChar * pszName, int iImage, void * pUserData);
   ~cToolItem();

   void SetState(uint mask, uint state);
   void ToggleChecked();

   cToolGroup * GetGroup() const;
   const tChar * GetName() const;
   uint GetState() const;
   int GetImageIndex() const;
   void * GetUserData() const;

   bool IsDisabled() const;
   bool IsChecked() const;

private:
   cToolGroup * m_pGroup;
   std::string m_name;
   uint m_state;
   int m_iImage;
   void * m_pUserData;
};

////////////////////////////////////////

inline cToolGroup * cToolItem::GetGroup() const
{
   return m_pGroup;
}

////////////////////////////////////////

inline const tChar * cToolItem::GetName() const
{
   return m_name.c_str();
}

////////////////////////////////////////

inline uint cToolItem::GetState() const
{
   return m_state;
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

////////////////////////////////////////

inline bool cToolItem::IsDisabled() const
{
   return (m_state & kTPTS_Disabled) == kTPTS_Disabled;
}

////////////////////////////////////////

inline bool cToolItem::IsChecked() const
{
   return (m_state & kTPTS_Checked) == kTPTS_Checked;
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

   bool IsCollapsed() const;
   void ToggleExpandCollapse();

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

   bool m_bCollapsed;
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

////////////////////////////////////////

inline bool cToolGroup::IsCollapsed() const
{
   return m_bCollapsed;
}

////////////////////////////////////////

inline void cToolGroup::ToggleExpandCollapse()
{
   m_bCollapsed = !m_bCollapsed;
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
   static int RenderGroupHeading(CDCHandle dc, LPRECT pRect, const cToolGroup * pGroup);

   typedef std::map<HANDLE, CRect> tCachedRects;
   tCachedRects m_cachedRects;

   CBrush m_checkedItemBrush;

   // These member variables valid only between Begin() and End() calls
   CDCHandle m_dc;
   CRect m_rect;
   CPoint m_mousePos;
   bool m_bHaveMousePos;
   int m_totalHeight;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPalette
//

typedef CWinTraitsOR<kTPS_ExclusiveCheck, 0> tToolPaleteWinTraits;

class cToolPalette : public CScrollWindowImpl<cToolPalette, CWindow, tToolPaleteWinTraits>,
                     public cTrackMouseEvent<cToolPalette>
{
   typedef CScrollWindowImpl<cToolPalette, CWindow, tToolPaleteWinTraits> tBase;
public:
   cToolPalette();
   ~cToolPalette();

   DECLARE_WND_CLASS("ToolPalette")

   BEGIN_MSG_MAP_EX(cToolPalette)
      CHAIN_MSG_MAP(cTrackMouseEvent<cToolPalette>)
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
      CHAIN_MSG_MAP(tBase)
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

   bool ExclusiveCheck() const;

   HTOOLGROUP AddGroup(const tChar * pszGroup, HIMAGELIST hImageList);
   bool RemoveGroup(HTOOLGROUP hGroup);
   HTOOLGROUP FindGroup(const tChar * pszGroup);
   bool IsGroup(HTOOLGROUP hGroup);
   bool IsTool(HTOOLITEM hTool);
   void Clear();
   HTOOLITEM AddTool(HTOOLGROUP hGroup, const tChar * pszTool, int iImage, void * pUserData = NULL);
   HTOOLITEM AddTool(HTOOLGROUP hGroup, const sToolPaletteItem * pTPI);
   bool GetToolText(HTOOLITEM hTool, std::string * pText);
   bool GetTool(HTOOLITEM hTool, sToolPaletteItem * pTPI);
   bool RemoveTool(HTOOLITEM hTool);
   bool EnableTool(HTOOLITEM hTool, bool bEnable);

private:
   void SetMouseOverItem(HANDLE hItem);
   void GetCheckedItems(std::vector<HTOOLITEM> * pCheckedItems);
   void DoClick(HANDLE hItem, CPoint point);
   void DoGroupClick(HTOOLGROUP hGroup, CPoint point);
   void DoItemClick(HTOOLITEM hTool, CPoint point);
   LRESULT DoNotify(cToolItem * pTool, int code, CPoint point = CPoint());

   tGroups m_groups;

   cToolPaletteRenderer m_renderer;

   CFont m_font;

   HANDLE m_hMouseOverItem;
   HANDLE m_hClickCandidateItem;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_TOOLPALETTE_H)
