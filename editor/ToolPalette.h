/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TOOLPALETTE_H)
#define INCLUDED_TOOLPALETTE_H

#include <string>
#include <vector>

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
   cToolItem(const tChar * pszName, int iImage);
   ~cToolItem();

   const tChar * GetName() const;
   int GetImageIndex() const;

private:
   std::string m_name;
   int m_iImage;
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

   HTOOLITEM AddTool(const tChar * pszTool, int iImage);
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
// CLASS: cToolGroupRenderer
//

class cToolGroupRenderer
{
public:
   cToolGroupRenderer(HDC hDC, LPCRECT pRect);
   cToolGroupRenderer(const cToolGroupRenderer &);
   ~cToolGroupRenderer();
   const cToolGroupRenderer & operator =(const cToolGroupRenderer &);

   void Render(const cToolGroup * pGroup);

   inline void operator ()(const cToolGroup * pGroup) { Render(pGroup); }

private:
   int RenderGroupHeading(const cToolGroup * pGroup);

   CDCHandle m_dc;
   CRect m_rect;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPalette
//

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
      MSG_WM_SETFONT(OnSetFont)
      MSG_WM_PAINT(OnPaint)
      MSG_WM_MOUSEMOVE(OnMouseMove)
      MSG_WM_LBUTTONDOWN(OnLButtonDown)
      MSG_WM_LBUTTONUP(OnLButtonUp)
   END_MSG_MAP()

   LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
   void OnDestroy();
   void OnSize(UINT nType, CSize size);
   void OnSetFont(HFONT hFont, BOOL bRedraw);
   void OnPaint(CDCHandle dc);
   void OnMouseMove(UINT flags, CPoint point);
   void OnLButtonDown(UINT flags, CPoint point);
   void OnLButtonUp(UINT flags, CPoint point);

   HTOOLGROUP AddGroup(const tChar * pszGroup, HIMAGELIST hImageList);
   bool RemoveGroup(HTOOLGROUP hGroup);
   HTOOLGROUP FindGroup(const tChar * pszGroup);
   bool IsGroup(HTOOLGROUP hGroup);
   void Clear();
   HTOOLITEM AddTool(HTOOLGROUP hGroup, const tChar * pszTool, int iImage);
   bool RemoveTool(HTOOLITEM hTool);
   bool EnableTool(HTOOLITEM hTool);

private:
   typedef std::vector<cToolGroup *> tGroups;
   tGroups m_groups;

   CFont m_font;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_TOOLPALETTE_H)
