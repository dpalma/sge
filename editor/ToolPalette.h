/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TOOLPALETTE_H)
#define INCLUDED_TOOLPALETTE_H

#include "TrackMouseEvent.h"

#include <atlgdi.h>
#include <atlcrack.h>
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
// Handles painting for cToolPaletteImpl. Called in a loop for each group to be
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

   int GetTotalHeight() const { return m_totalHeight; }

   inline void FlushCachedRects() { m_cachedRects.clear(); }

   inline void operator ()(const cToolGroup * pGroup) { Render(pGroup); }

private:
   static int RenderGroupHeading(WTL::CDCHandle dc, LPRECT pRect, const cToolGroup * pGroup);

   typedef std::map<HANDLE, CRect> tCachedRects;
   tCachedRects m_cachedRects;

   WTL::CBrush m_checkedItemBrush;

   // These member variables valid only between Begin() and End() calls
   WTL::CDCHandle m_dc;
   CRect m_rect;
   CPoint m_mousePos;
   bool m_bHaveMousePos;
   int m_totalHeight;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteImpl
//

typedef CWinTraitsOR<kTPS_ExclusiveCheck, 0> tToolPaleteWinTraits;

template <class T, class TBase = CWindow, class TWinTraits = tToolPaleteWinTraits>
class ATL_NO_VTABLE cToolPaletteImpl : public WTL::CScrollWindowImpl<T, TBase, TWinTraits>,
                                       public cTrackMouseEvent<T>
{
   typedef WTL::CScrollWindowImpl<T, TBase, TWinTraits> tBase;

public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

   cToolPaletteImpl();
   ~cToolPaletteImpl();

   HTOOLGROUP AddGroup(const tChar * pszGroup, HIMAGELIST hImageList);
   bool RemoveGroup(HTOOLGROUP hGroup);
   uint GetGroupCount() const;
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

   // CScrollWindowImpl handles WM_PAINT and delegates to this method
   void DoPaint(WTL::CDCHandle dc);

protected:
   BEGIN_MSG_MAP_EX(cToolPaletteImpl)
      CHAIN_MSG_MAP(tBase)
      CHAIN_MSG_MAP(cTrackMouseEvent<T>)
      MSG_WM_CREATE(OnCreate)
      MSG_WM_DESTROY(OnDestroy)
      MSG_WM_SIZE(OnSize)
      MSG_WM_SETFONT(OnSetFont)
      MSG_WM_ERASEBKGND(OnEraseBkgnd)
      MSG_WM_MOUSELEAVE(OnMouseLeave)
      // Adjust mouse position for scrolling for all mouse messages
      if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
      {
         CPoint ptAdjusted(lParam);
         ptAdjusted += m_ptOffset;
         lParam = MAKELPARAM(ptAdjusted.x, ptAdjusted.y);
         MSG_WM_MOUSEMOVE(OnMouseMove)
         MSG_WM_LBUTTONDOWN(OnLButtonDown)
         MSG_WM_LBUTTONUP(OnLButtonUp)
      }
   END_MSG_MAP()

   LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
   void OnDestroy();
   void OnSize(UINT nType, CSize size);
   void OnSetFont(HFONT hFont, BOOL bRedraw);
   LRESULT OnEraseBkgnd(WTL::CDCHandle dc);
   void OnMouseLeave();
   void OnMouseMove(UINT flags, CPoint point);
   void OnLButtonDown(UINT flags, CPoint point);
   void OnLButtonUp(UINT flags, CPoint point);

private:
   void GetCheckedItems(std::vector<HTOOLITEM> * pCheckedItems);

   void SetMouseOverItem(HANDLE hItem);

   void DoClick(HANDLE hItem, CPoint point);
   void DoGroupClick(HTOOLGROUP hGroup, CPoint point);
   void DoItemClick(HTOOLITEM hTool, CPoint point);
   LRESULT DoNotify(cToolItem * pTool, int code, CPoint point = CPoint());

   bool ExclusiveCheck() const
   {
      return (GetStyle() & kTPS_ExclusiveCheck) == kTPS_ExclusiveCheck;
   }

   ////////////////////////////////////

   tGroups m_groups;

   cToolPaletteRenderer m_renderer;

   bool m_bUpdateScrollInfo;

   WTL::CFont m_font;

   HANDLE m_hMouseOverItem;
   HANDLE m_hClickCandidateItem;
};

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
cToolPaletteImpl<T, TBase, TWinTraits>::cToolPaletteImpl()
 : m_bUpdateScrollInfo(false),
   m_hMouseOverItem(NULL),
   m_hClickCandidateItem(NULL)
{
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
cToolPaletteImpl<T, TBase, TWinTraits>::~cToolPaletteImpl()
{
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
HTOOLGROUP cToolPaletteImpl<T, TBase, TWinTraits>::AddGroup(const tChar * pszGroup, HIMAGELIST hImageList)
{
   if (pszGroup != NULL)
   {
      HTOOLGROUP hGroup = FindGroup(pszGroup);
      if (hGroup != NULL)
      {
         return hGroup;
      }

      cToolGroup * pGroup = new cToolGroup(pszGroup, hImageList);
      if (pGroup != NULL)
      {
         m_bUpdateScrollInfo = true;
         m_groups.push_back(pGroup);
         return reinterpret_cast<HTOOLGROUP>(pGroup);
      }
   }

   return NULL;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
bool cToolPaletteImpl<T, TBase, TWinTraits>::RemoveGroup(HTOOLGROUP hGroup)
{
   if (hGroup != NULL)
   {
      cToolGroup * pRmGroup = reinterpret_cast<cToolGroup *>(hGroup);
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if (*iter == pRmGroup)
         {
            uint nTools = pRmGroup->GetToolCount();
            for (uint i = 0; i < nTools; i++)
            {
               DoNotify(pRmGroup->GetTool(i), kTPN_ItemDestroy);
            }
            delete pRmGroup;
            m_groups.erase(iter);
            m_renderer.FlushCachedRects();
            m_bUpdateScrollInfo = true;
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
uint cToolPaletteImpl<T, TBase, TWinTraits>::GetGroupCount() const
{
   return m_groups.size();
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
HTOOLGROUP cToolPaletteImpl<T, TBase, TWinTraits>::FindGroup(const tChar * pszGroup)
{
   if (pszGroup != NULL)
   {
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if (lstrcmp((*iter)->GetName(), pszGroup) == 0)
         {
            return reinterpret_cast<HTOOLGROUP>(*iter);
         }
      }
   }
   return NULL;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
bool cToolPaletteImpl<T, TBase, TWinTraits>::IsGroup(HTOOLGROUP hGroup)
{
   if (hGroup != NULL)
   {
      cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if (*iter == pGroup)
         {
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
bool cToolPaletteImpl<T, TBase, TWinTraits>::IsTool(HTOOLITEM hTool)
{
   if (hTool != NULL)
   {
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if ((*iter)->IsTool(hTool))
         {
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::Clear()
{
   // Call RemoveGroup() for each so that all the item destroy notifications happen
   while (!m_groups.empty())
   {
      RemoveGroup(reinterpret_cast<HTOOLGROUP>(m_groups.front()));
   }

   // Still important to let the STL container do any internal cleanup
   m_groups.clear();

   m_bUpdateScrollInfo = true;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
HTOOLITEM cToolPaletteImpl<T, TBase, TWinTraits>::AddTool(HTOOLGROUP hGroup, const tChar * pszTool, int iImage, void * pUserData)
{
   sToolPaletteItem tpi = {0};
   lstrcpyn(tpi.szName, pszTool, _countof(tpi.szName));
   tpi.iImage = iImage;
   tpi.pUserData = pUserData;
   return AddTool(hGroup, &tpi);
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
HTOOLITEM cToolPaletteImpl<T, TBase, TWinTraits>::AddTool(HTOOLGROUP hGroup, const sToolPaletteItem * pTPI)
{
   if (IsGroup(hGroup) && (pTPI != NULL))
   {
      cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
      HTOOLITEM hItem = pGroup->AddTool(pTPI->szName, pTPI->iImage, pTPI->pUserData);
      if (hItem != NULL)
      {
         m_bUpdateScrollInfo = true;
         Invalidate();
         return hItem;
      }
   }

   return NULL;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
bool cToolPaletteImpl<T, TBase, TWinTraits>::GetToolText(HTOOLITEM hTool, std::string * pText)
{
   if (IsTool(hTool))
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));

      if (pText != NULL)
      {
         *pText = pTool->GetName();
      }

      return true;
   }

   return false;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
bool cToolPaletteImpl<T, TBase, TWinTraits>::GetTool(HTOOLITEM hTool, sToolPaletteItem * pTPI)
{
   if (IsTool(hTool))
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));

      if (pTPI != NULL)
      {
         lstrcpyn(pTPI->szName, pTool->GetName(), _countof(pTPI->szName));
         pTPI->iImage = pTool->GetImageIndex();
         pTPI->state = 0;
         pTPI->pUserData = pTool->GetUserData();
         return true;
      }
   }

   return false;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
bool cToolPaletteImpl<T, TBase, TWinTraits>::RemoveTool(HTOOLITEM hTool)
{
   // TODO
   WarnMsg("Unsupported function called: cToolPalette::RemoveTool\n");
   return false;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
bool cToolPaletteImpl<T, TBase, TWinTraits>::EnableTool(HTOOLITEM hTool, bool bEnable)
{
   if (IsTool(hTool))
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));

      pTool->SetState(kTPTS_Disabled, bEnable ? 0 : kTPTS_Disabled);

      return true;
   }

   return false;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::DoPaint(WTL::CDCHandle dc)
{
   CRect rect;
   GetClientRect(rect);

   const POINT * pMousePos = NULL;
   POINT mousePos;
   if (GetCursorPos(&mousePos) && ::MapWindowPoints(NULL, m_hWnd, &mousePos, 1))
   {
      pMousePos = &mousePos;
   }

   HFONT hOldFont = dc.SelectFont(!m_font.IsNull() ? m_font : WTL::AtlGetDefaultGuiFont());

   Verify(m_renderer.Begin(dc, rect, pMousePos));
   m_renderer.Render(m_groups.begin(), m_groups.end());
   Verify(m_renderer.End());

   dc.SelectFont(hOldFont);

   if (m_bUpdateScrollInfo)
   {
      m_bUpdateScrollInfo = false;
      SetScrollSize(1, m_renderer.GetTotalHeight());
   }
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
LRESULT cToolPaletteImpl<T, TBase, TWinTraits>::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   return 0;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::OnDestroy()
{
   Clear();
   SetFont(NULL, FALSE);
   m_renderer.FlushCachedRects();
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::OnSize(UINT nType, CSize size)
{
   m_bUpdateScrollInfo = true;
   m_renderer.FlushCachedRects();
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::OnSetFont(HFONT hFont, BOOL bRedraw)
{
   if (!m_font.IsNull())
   {
      Verify(m_font.DeleteObject());
   }

   if (hFont != NULL)
   {
      LOGFONT logFont = {0};
      if (GetObject(hFont, sizeof(LOGFONT), &logFont))
      {
         m_font.CreateFontIndirect(&logFont);
      }
   }

   if (bRedraw)
   {
      Invalidate();
      UpdateWindow();
   }
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
LRESULT cToolPaletteImpl<T, TBase, TWinTraits>::OnEraseBkgnd(WTL::CDCHandle dc)
{
   return TRUE;
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::OnMouseLeave()
{
   SetMouseOverItem(NULL);
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::OnMouseMove(UINT flags, CPoint point)
{
   SetMouseOverItem(m_renderer.GetHitItem(point));
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::OnLButtonDown(UINT flags, CPoint point)
{
   Assert(m_hClickCandidateItem == NULL);
   m_hClickCandidateItem = m_renderer.GetHitItem(point);
   if (m_hClickCandidateItem != NULL)
   {
      SetCapture();
   }
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::OnLButtonUp(UINT flags, CPoint point)
{
   if (m_hClickCandidateItem != NULL)
   {
      ReleaseCapture();
      if (m_hClickCandidateItem == m_renderer.GetHitItem(point))
      {
         DoClick(m_hClickCandidateItem, point);
      }
      m_hClickCandidateItem = NULL;
   }
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::GetCheckedItems(std::vector<HTOOLITEM> * pCheckedItems)
{
   if (pCheckedItems != NULL)
   {
      pCheckedItems->clear();

      tGroups::const_iterator iter = m_groups.begin();
      tGroups::const_iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         uint nTools = (*iter)->GetToolCount();
         for (uint i = 0; i < nTools; i++)
         {
            cToolItem * pTool = (*iter)->GetTool(i);
            if (pTool->IsChecked())
            {
               pCheckedItems->push_back(reinterpret_cast<HTOOLITEM>(pTool));
            }
         }
      }
   }
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::SetMouseOverItem(HANDLE hItem)
{
   if (hItem != m_hMouseOverItem)
   {
      // remove this if-statement if group headings ever have a mouse-over effect
      if (!IsGroup(reinterpret_cast<HTOOLGROUP>(m_hMouseOverItem)))
      {
         CRect oldItemRect;
         if ((m_hMouseOverItem != NULL) && m_renderer.GetItemRect(m_hMouseOverItem, &oldItemRect))
         {
            InvalidateRect(oldItemRect, TRUE);
         }
      }

      m_hMouseOverItem = hItem;

      // remove this if-statement if group headings ever have a mouse-over effect
      if (!IsGroup(reinterpret_cast<HTOOLGROUP>(hItem)))
      {
         CRect itemRect;
         if ((hItem != NULL) && m_renderer.GetItemRect(hItem, &itemRect))
         {
            InvalidateRect(itemRect, FALSE);
         }
      }
   }
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::DoClick(HANDLE hItem, CPoint point)
{
   if (hItem != NULL)
   {
      if (IsGroup(reinterpret_cast<HTOOLGROUP>(hItem)))
      {
         DoGroupClick(reinterpret_cast<HTOOLGROUP>(hItem), point);
      }
      else
      {
         DoItemClick(reinterpret_cast<HTOOLITEM>(hItem), point);
      }
   }
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::DoGroupClick(HTOOLGROUP hGroup, CPoint point)
{
   cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
   pGroup->ToggleExpandCollapse();
   m_bUpdateScrollInfo = true;
   RECT rg;
   if (m_renderer.GetItemRect(hGroup, &rg))
   {
      m_renderer.FlushCachedRects();
      RECT rc;
      GetClientRect(&rc);
      rg.bottom = rc.bottom;
      InvalidateRect(&rg);
   }
   else
   {
      m_renderer.FlushCachedRects();
      Invalidate();
   }
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
void cToolPaletteImpl<T, TBase, TWinTraits>::DoItemClick(HTOOLITEM hTool, CPoint point)
{
   cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
   Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));
   if (!pTool->IsDisabled())
   {
      DoNotify(pTool, kTPN_ItemClick, point);
      if (ExclusiveCheck())
      {
         std::vector<HTOOLITEM> checkedItems;
         GetCheckedItems(&checkedItems);
         Assert(checkedItems.size() <= 1);
         if (!checkedItems.empty())
         {
            if (checkedItems[0] != hTool)
            {
               cToolItem * pUncheckTool = reinterpret_cast<cToolItem *>(checkedItems[0]);

               DoNotify(pUncheckTool, kTPN_ItemUncheck);
               pUncheckTool->SetState(kTPTS_Checked, 0);

               RECT r;
               if (m_renderer.GetItemRect(checkedItems[0], &r))
               {
                  InvalidateRect(&r);
               }
               else
               {
                  Invalidate();
               }
            }
         }

         if (!pTool->IsChecked())
         {
            DoNotify(pTool, kTPN_ItemCheck);
            pTool->SetState(kTPTS_Checked, kTPTS_Checked);
         }
      }
      else
      {
         DoNotify(pTool, pTool->IsChecked()? kTPN_ItemUncheck : kTPN_ItemCheck);
         pTool->ToggleChecked();
      }
      RECT rt;
      if (m_renderer.GetItemRect(hTool, &rt))
      {
         InvalidateRect(&rt);
      }
      else
      {
         Invalidate();
      }
   }
}

////////////////////////////////////////

template <class T, class TBase, class TWinTraits>
LRESULT cToolPaletteImpl<T, TBase, TWinTraits>::DoNotify(cToolItem * pTool, int code, CPoint point)
{
   Assert(code == kTPN_ItemClick || code == kTPN_ItemDestroy
      || code == kTPN_ItemCheck || code == kTPN_ItemUncheck);
   if (pTool != NULL)
   {
      HWND hWndParent = GetParent();
      if (::IsWindow(hWndParent))
      {
         sNMToolPaletteItem nm = {0};
         nm.hdr.hwndFrom = m_hWnd;
         nm.hdr.code = code;
         nm.hdr.idFrom = GetDlgCtrlID();
         nm.pt = point;
         nm.hTool = reinterpret_cast<HTOOLITEM>(pTool);
         nm.pUserData = pTool->GetUserData();
         return ::SendMessage(hWndParent, WM_NOTIFY, nm.hdr.idFrom, reinterpret_cast<LPARAM>(&nm));
      }
   }
   return -1;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPalette
//

class cToolPalette : public cToolPaletteImpl<cToolPalette>
{
public:
	DECLARE_WND_SUPERCLASS(_T("ToolPalette"), GetWndClassName())
};


/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_TOOLPALETTE_H)
