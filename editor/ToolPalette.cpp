/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPalette.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static const int kTextGap = 2;

///////////////////////////////////////////////////////////////////////////////

static byte GrayLevel(COLORREF color)
{
   double intensity =
      0.2989 * ((double)GetRValue(color) / 255) +
      0.5870 * ((double)GetGValue(color) / 255) +
      0.1140 * ((double)GetBValue(color) / 255);

   if (intensity < 0)
   {
      intensity = 0;
   }
   else if (intensity > 1)
   {
      intensity = 1;
   }

   return (byte)(intensity * 255);
}

///////////////////////////////////////////////////////////////////////////////

#define GetRValue16(color) ((uint16)(GetRValue((color))<<8))
#define GetGValue16(color) ((uint16)(GetGValue((color))<<8))
#define GetBValue16(color) ((uint16)(GetBValue((color))<<8))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDynamicLink
//

class cDynamicLink
{
public:
   cDynamicLink(const tChar * pszLibrary);
   ~cDynamicLink();

   FARPROC GetProcAddress(const tChar * pszProc);

private:
   std::string m_name;
   HINSTANCE m_hLibrary;
};

////////////////////////////////////////

cDynamicLink::cDynamicLink(const tChar * pszLibrary)
 : m_name(pszLibrary != NULL ? pszLibrary : ""),
   m_hLibrary(NULL)
{
}

////////////////////////////////////////

cDynamicLink::~cDynamicLink()
{
   if (m_hLibrary != NULL)
   {
      FreeLibrary(m_hLibrary);
      m_hLibrary = NULL;
   }
}

////////////////////////////////////////

FARPROC cDynamicLink::GetProcAddress(const tChar * pszProc)
{
   if (m_hLibrary == NULL)
   {
      if (m_name.empty())
      {
         return NULL;
      }

      m_hLibrary = LoadLibrary(m_name.c_str());
      if (m_hLibrary == NULL)
      {
         return NULL;
      }
   }

   return ::GetProcAddress(m_hLibrary, pszProc);
}


///////////////////////////////////////////////////////////////////////////////

static cDynamicLink g_msimg32("msimg32");

typedef BOOL (WINAPI * tGradientFillFn)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);

BOOL WINAPI DynamicGradientFill(HDC hdc,
                                PTRIVERTEX pVertex,
                                ULONG dwNumVertex,
                                PVOID pMesh,
                                ULONG dwNumMesh,
                                ULONG dwMode)
{
   static bool bTriedAndFailed = false;
   static tGradientFillFn pfnGradientFill = NULL;

   if (pfnGradientFill == NULL && !bTriedAndFailed)
   {
      pfnGradientFill = reinterpret_cast<tGradientFillFn>(g_msimg32.GetProcAddress("GradientFill"));
      if (pfnGradientFill == NULL)
      {
         bTriedAndFailed = true;
      }
   }

   if (pfnGradientFill != NULL)
   {
      return (*pfnGradientFill)(hdc, pVertex, dwNumVertex, pMesh, dwNumMesh, dwMode);
   }
   else
   {
      return FALSE;
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolItem
//

////////////////////////////////////////

cToolItem::cToolItem(const tChar * pszName, int iImage)
 : m_name(pszName != NULL ? pszName : ""),
   m_iImage(iImage)
{
}

////////////////////////////////////////

cToolItem::~cToolItem()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolGroup
//

////////////////////////////////////////

cToolGroup::cToolGroup(const tChar * pszName, HIMAGELIST hImageList)
 : m_name(pszName != NULL ? pszName : ""),
   m_hImageList(hImageList)
{
}

////////////////////////////////////////

cToolGroup::~cToolGroup()
{
   if (m_hImageList != NULL)
   {
      ImageList_Destroy(m_hImageList);
      m_hImageList = NULL;
   }

   Clear();
}

////////////////////////////////////////

HTOOLITEM cToolGroup::AddTool(const tChar * pszTool, int iImage)
{
   if (pszTool != NULL)
   {
      HTOOLITEM hTool = FindTool(pszTool);
      if (hTool != NULL)
      {
         return hTool;
      }

      cToolItem * pTool = new cToolItem(pszTool, iImage);
      if (pTool != NULL)
      {
         m_tools.push_back(pTool);
         return reinterpret_cast<HTOOLITEM>(pTool);
      }
   }

   return NULL;
}

////////////////////////////////////////

bool cToolGroup::RemoveTool(HTOOLITEM hTool)
{
   if (hTool != NULL)
   {
      cToolItem * pRmTool = reinterpret_cast<cToolItem *>(hTool);
      tTools::iterator iter = m_tools.begin();
      tTools::iterator end = m_tools.end();
      for (; iter != end; iter++)
      {
         if (*iter == pRmTool)
         {
            delete pRmTool;
            m_tools.erase(iter);
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

HTOOLITEM cToolGroup::FindTool(const tChar * pszTool)
{
   if (pszTool != NULL)
   {
      tTools::iterator iter = m_tools.begin();
      tTools::iterator end = m_tools.end();
      for (; iter != end; iter++)
      {
         if (lstrcmp((*iter)->GetName(), pszTool) == 0)
         {
            return reinterpret_cast<HTOOLITEM>(*iter);
         }
      }
   }
   return NULL;
}

////////////////////////////////////////

bool cToolGroup::IsTool(HTOOLITEM hTool) const
{
   if (hTool != NULL)
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      tTools::const_iterator iter = m_tools.begin();
      tTools::const_iterator end = m_tools.end();
      for (; iter != end; iter++)
      {
         if (*iter == pTool)
         {
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

void cToolGroup::Clear()
{
   tTools::iterator iter = m_tools.begin();
   tTools::iterator end = m_tools.end();
   for (; iter != end; iter++)
   {
      delete *iter;
   }
   m_tools.clear();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolGroupRenderer
//

////////////////////////////////////////

cToolGroupRenderer::cToolGroupRenderer(HDC hDC, LPCRECT pRect)
 : m_dc(hDC),
   m_rect(pRect)
{
}

////////////////////////////////////////

cToolGroupRenderer::cToolGroupRenderer(const cToolGroupRenderer & other)
 : m_dc(other.m_dc),
   m_rect(other.m_rect)
{
}

////////////////////////////////////////

cToolGroupRenderer::~cToolGroupRenderer()
{
}

////////////////////////////////////////

const cToolGroupRenderer & cToolGroupRenderer::operator =(const cToolGroupRenderer & other)
{
   m_dc = other.m_dc;
   m_rect = other.m_rect;
   return *this;
}

////////////////////////////////////////

void cToolGroupRenderer::Render(const cToolGroup * pGroup)
{
   if (pGroup == NULL)
   {
      return;
   }

   int headingHeight = RenderGroupHeading(pGroup);

   HIMAGELIST hImageList = pGroup->GetImageList();

   CRect toolRect(m_rect);
   toolRect.top += headingHeight;

   uint nTools = pGroup->GetToolCount();
   for (uint i = 0; i < nTools; i++)
   {
      cToolItem * pTool = pGroup->GetTool(i);
      if (pTool != NULL)
      {
         CSize extent;
         m_dc.GetTextExtent(pTool->GetName(), -1, &extent);

         extent.cy += (2 * kTextGap);

         toolRect.bottom = toolRect.top + extent.cy;

         int iImage = pTool->GetImageIndex();

         if ((hImageList != NULL) && (iImage > -1))
         {
            // TODO: vertically center
            ImageList_Draw(hImageList, iImage, m_dc, toolRect.left, toolRect.top, 0);
         }

         CRect r(toolRect);
         r.left += kTextGap;

         m_dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
         m_dc.SetBkMode(OPAQUE);
         m_dc.DrawText(pTool->GetName(), -1, &r, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

         toolRect.OffsetRect(0, toolRect.Height());
      }
   }
}

////////////////////////////////////////

int cToolGroupRenderer::RenderGroupHeading(const cToolGroup * pGroup)
{
   if ((pGroup == NULL) || (lstrlen(pGroup->GetName()) == 0))
   {
      return 0;
   }

   Assert(!m_dc.IsNull());

   CSize extent;
   m_dc.GetTextExtent(pGroup->GetName(), -1, &extent);

   COLORREF leftColor = GetSysColor(COLOR_3DSHADOW);
   COLORREF rightColor = GetSysColor(COLOR_3DFACE);

   extent.cy += (2 * kTextGap);

   CRect r(m_rect);
   r.bottom = r.top + extent.cy;
   m_dc.Draw3dRect(r, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));

   r.DeflateRect(1, 1);

   int halfWidth = r.Width() / 2;

   TRIVERTEX gv[2];
   gv[0].x = r.left;
   gv[0].y = r.top;
   gv[0].Red = GetRValue16(leftColor);
   gv[0].Green = GetGValue16(leftColor);
   gv[0].Blue = GetBValue16(leftColor);
   gv[0].Alpha = 0;
   gv[1].x = r.left + halfWidth;
   gv[1].y = r.bottom; 
   gv[1].Red = GetRValue16(rightColor);
   gv[1].Green = GetGValue16(rightColor);
   gv[1].Blue = GetBValue16(rightColor);
   gv[1].Alpha = 0;

   GRADIENT_RECT gr;
   gr.UpperLeft = 0;
   gr.LowerRight = 1;
   DynamicGradientFill(m_dc, gv, _countof(gv), &gr, 1, GRADIENT_FILL_RECT_H);

   m_dc.FillSolidRect(r.left + halfWidth, r.top, halfWidth, r.Height(), rightColor);

   r.left += kTextGap;

   m_dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
   m_dc.SetBkMode(TRANSPARENT);
   m_dc.DrawText(pGroup->GetName(), -1, &r, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

   return extent.cy;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPalette
//

////////////////////////////////////////

cToolPalette::cToolPalette()
{
}

////////////////////////////////////////

cToolPalette::~cToolPalette()
{
}

////////////////////////////////////////

LRESULT cToolPalette::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   return 0;
}

////////////////////////////////////////

void cToolPalette::OnDestroy()
{
   Clear();
   SetFont(NULL, FALSE);
}

////////////////////////////////////////

void cToolPalette::OnSize(UINT nType, CSize size)
{
}

////////////////////////////////////////

void cToolPalette::OnSetFont(HFONT hFont, BOOL bRedraw)
{
   if (!m_font.IsNull())
   {
      m_font.DeleteObject();
   }

   if (hFont != NULL)
   {
      LOGFONT logFont = {0};
      if (GetObject(hFont, sizeof(LOGFONT), &logFont))
      {
         m_font.CreateFontIndirect(&logFont);
      }
   }
}

////////////////////////////////////////

void cToolPalette::OnPaint(CDCHandle dc)
{
   CPaintDC paintDC(m_hWnd);

   CRect rect;
   GetClientRect(rect);

   HFONT hOldFont = paintDC.SelectFont(!m_font.IsNull() ? m_font : AtlGetDefaultGuiFont());

   cToolGroupRenderer groupRenderer(paintDC, rect);
   std::for_each(m_groups.begin(), m_groups.end(), groupRenderer);

   paintDC.SelectFont(hOldFont);
}

////////////////////////////////////////

void cToolPalette::OnMouseMove(UINT flags, CPoint point)
{
}

////////////////////////////////////////

void cToolPalette::OnLButtonDown(UINT flags, CPoint point)
{
}

////////////////////////////////////////

void cToolPalette::OnLButtonUp(UINT flags, CPoint point)
{
}

////////////////////////////////////////

HTOOLGROUP cToolPalette::AddGroup(const tChar * pszGroup, HIMAGELIST hImageList)
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
         m_groups.push_back(pGroup);
         return reinterpret_cast<HTOOLGROUP>(pGroup);
      }
   }

   return NULL;
}

////////////////////////////////////////

bool cToolPalette::RemoveGroup(HTOOLGROUP hGroup)
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
            delete pRmGroup;
            m_groups.erase(iter);
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

HTOOLGROUP cToolPalette::FindGroup(const tChar * pszGroup)
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

bool cToolPalette::IsGroup(HTOOLGROUP hGroup)
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

void cToolPalette::Clear()
{
   tGroups::iterator iter = m_groups.begin();
   tGroups::iterator end = m_groups.end();
   for (; iter != end; iter++)
   {
      delete *iter;
   }
   m_groups.clear();
}

////////////////////////////////////////

HTOOLITEM cToolPalette::AddTool(HTOOLGROUP hGroup, const tChar * pszTool, int iImage)
{
   if (IsGroup(hGroup))
   {
      cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
      HTOOLITEM hItem = pGroup->AddTool(pszTool, iImage);
      if (hItem != NULL)
      {
         Invalidate();
         return hItem;
      }
   }

   return NULL;
}

////////////////////////////////////////

bool cToolPalette::RemoveTool(HTOOLITEM hTool)
{
   // TODO
   return false;
}

////////////////////////////////////////

bool cToolPalette::EnableTool(HTOOLITEM hTool)
{
   // TODO
   return false;
}

///////////////////////////////////////////////////////////////////////////////
