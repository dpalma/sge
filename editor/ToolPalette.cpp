/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPalette.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static const int kTextGap = 2;
static const int kImageGap = 1;

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
// CLASS: cToolPaletteRenderer
//

////////////////////////////////////////

cToolPaletteRenderer::cToolPaletteRenderer()
 : m_bHaveMousePos(false)
{
}

////////////////////////////////////////

cToolPaletteRenderer::cToolPaletteRenderer(const cToolPaletteRenderer & other)
 : m_dc(other.m_dc),
   m_rect(other.m_rect),
   m_mousePos(other.m_mousePos),
   m_bHaveMousePos(other.m_bHaveMousePos)
{
}

////////////////////////////////////////

cToolPaletteRenderer::~cToolPaletteRenderer()
{
}

////////////////////////////////////////

const cToolPaletteRenderer & cToolPaletteRenderer::operator =(const cToolPaletteRenderer & other)
{
   m_dc = other.m_dc;
   m_rect = other.m_rect;
   m_mousePos = other.m_mousePos;
   m_bHaveMousePos = other.m_bHaveMousePos;
   return *this;
}

////////////////////////////////////////

bool cToolPaletteRenderer::Begin(HDC hDC, LPCRECT pRect, const POINT * pMousePos)
{
   if (m_dc.IsNull() && (hDC != NULL) && (pRect != NULL))
   {
      m_dc = hDC;
      m_rect = pRect;
      if (pMousePos != NULL)
      {
         m_mousePos = *pMousePos;
         m_bHaveMousePos = true;
      }
      else
      {
         m_bHaveMousePos = false;
      }
      return true;
   }
   return false;
}

////////////////////////////////////////

bool cToolPaletteRenderer::End()
{
   if (!m_dc.IsNull())
   {
      m_dc = NULL;
      m_bHaveMousePos = false;
      return true;
   }
   return false;
}

////////////////////////////////////////

HANDLE cToolPaletteRenderer::GetHitItem(const CPoint & point, RECT * pRect)
{
   tCachedRects::const_iterator iter = m_cachedRects.begin();
   tCachedRects::const_iterator end = m_cachedRects.end();
   for (; iter != end; iter++)
   {
      if (iter->second.PtInRect(point))
      {
         if (pRect != NULL)
         {
            *pRect = iter->second;
         }
         return iter->first;
      }
   }
   return NULL;
}

////////////////////////////////////////

bool cToolPaletteRenderer::GetItemRect(HANDLE hItem, RECT * pRect)
{
   tCachedRects::const_iterator iter = m_cachedRects.find(hItem);
   if (iter != m_cachedRects.end())
   {
      if (pRect != NULL)
      {
         *pRect = iter->second;
      }
      return true;
   }
   return false;
}

////////////////////////////////////////

void cToolPaletteRenderer::Render(tGroups::const_iterator from,
                                  tGroups::const_iterator to)
{
   if (m_dc.IsNull())
   {
      return;
   }

   // Not using std::for_each() here because the functor is passed by
   // value which instantiates a temporary object and the renderer class
   // is a little too heavy to copy on every paint.
   tGroups::const_iterator iter = from;
   for (; iter != to; iter++)
   {
      Render(*iter);
   }
}

////////////////////////////////////////

void cToolPaletteRenderer::Render(const cToolGroup * pGroup)
{
   if (m_dc.IsNull() || (pGroup == NULL))
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

         CRect textRect(toolRect);
         CPoint imagePos(toolRect.TopLeft());

         if ((hImageList != NULL) && (iImage > -1))
         {
            IMAGEINFO imageInfo;
            if (ImageList_GetImageInfo(hImageList, iImage, &imageInfo))
            {
               int imageHeight = (imageInfo.rcImage.bottom - imageInfo.rcImage.top);
               int imageWidth = (imageInfo.rcImage.right - imageInfo.rcImage.left);
               if ((imageHeight + (2 * kImageGap)) > toolRect.Height())
               {
                  toolRect.bottom = toolRect.top + imageHeight + (2 * kImageGap);
               }
               imagePos.x = toolRect.left + kImageGap;
               imagePos.y = toolRect.top + ((toolRect.Height() - imageHeight) / 2);
               textRect.left = imagePos.x + imageWidth;
            }
            else
            {
               // ImageList_GetImageInfo() failed so don't draw the image
               iImage = -1;
            }
         }

         m_cachedRects[reinterpret_cast<HANDLE>(pTool)] = toolRect;

         textRect.left += kTextGap;

         // All size/position calculation done, now draw

         if (m_bHaveMousePos && toolRect.PtInRect(m_mousePos))
         {
            m_dc.Draw3dRect(toolRect, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
         }

         if ((hImageList != NULL) && (iImage > -1))
         {
            ImageList_Draw(hImageList, iImage, m_dc, imagePos.x, imagePos.y, 0);
         }

         COLORREF oldTextColor = m_dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
         int oldBkMode = m_dc.SetBkMode(TRANSPARENT);
         m_dc.DrawText(pTool->GetName(), -1, &textRect, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
         m_dc.SetTextColor(oldTextColor);
         m_dc.SetBkMode(oldBkMode);

         toolRect.OffsetRect(0, toolRect.Height());
      }
   }
}

////////////////////////////////////////

int cToolPaletteRenderer::RenderGroupHeading(const cToolGroup * pGroup)
{
   if ((pGroup == NULL) || (lstrlen(pGroup->GetName()) == 0))
   {
      return 0;
   }

   Assert(!m_dc.IsNull());

   CSize extent;
   m_dc.GetTextExtent(pGroup->GetName(), -1, &extent);

   extent.cy += (2 * kTextGap);

   CRect r(m_rect);
   r.bottom = r.top + extent.cy;

   m_dc.Draw3dRect(r, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));

   r.DeflateRect(1, 1);

   int halfWidth = r.Width() / 2;

   COLORREF leftColor = GetSysColor(COLOR_3DSHADOW);
   COLORREF rightColor = GetSysColor(COLOR_3DFACE);

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
 : m_bTrackingMouseLeave(false),
   m_hMouseOverItem(NULL)
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
   m_renderer.FlushCachedRects();
}

////////////////////////////////////////

void cToolPalette::OnSize(UINT nType, CSize size)
{
   m_renderer.FlushCachedRects();
}

////////////////////////////////////////

void cToolPalette::OnSetFont(HFONT hFont, BOOL bRedraw)
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

LRESULT cToolPalette::OnEraseBkgnd(CDCHandle dc)
{
   if (!dc.IsNull())
   {
      CRect rect;
      GetClientRect(rect);

      dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));

      return TRUE;
   }

   return FALSE;
}

////////////////////////////////////////

void cToolPalette::OnPaint(CDCHandle dc)
{
   CPaintDC paintDC(m_hWnd);

   CRect rect;
   GetClientRect(rect);

   const POINT * pMousePos = NULL;
   CPoint mousePos;
   if (GetCursorPos(&mousePos) && ::MapWindowPoints(NULL, m_hWnd, &mousePos, 1))
   {
      pMousePos = &mousePos;
   }

   HFONT hOldFont = paintDC.SelectFont(!m_font.IsNull() ? m_font : AtlGetDefaultGuiFont());

   Verify(m_renderer.Begin(paintDC, rect, pMousePos));
   m_renderer.Render(m_groups.begin(), m_groups.end());
   Verify(m_renderer.End());

   paintDC.SelectFont(hOldFont);
}

////////////////////////////////////////

void cToolPalette::OnMouseLeave()
{
   m_bTrackingMouseLeave = false;

   SetMouseOverItem(NULL);
}

////////////////////////////////////////

void cToolPalette::OnMouseMove(UINT flags, CPoint point)
{
   // TODO: refactor use of TrackMouseEvent out into a "mix-in" class
   // see cGLContext
   if (!m_bTrackingMouseLeave)
   {
      TRACKMOUSEEVENT tme = {0};
      tme.cbSize = sizeof(TRACKMOUSEEVENT);
      tme.dwFlags = TME_LEAVE;
      tme.hwndTrack = m_hWnd;
      if (_TrackMouseEvent(&tme))
      {
         m_bTrackingMouseLeave = true;
      }
   }

   SetMouseOverItem(m_renderer.GetHitItem(point));
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

////////////////////////////////////////

void cToolPalette::SetMouseOverItem(HANDLE hItem)
{
   if (hItem != m_hMouseOverItem)
   {
      CRect oldItemRect;
      if ((m_hMouseOverItem != NULL) && m_renderer.GetItemRect(m_hMouseOverItem, &oldItemRect))
      {
         InvalidateRect(oldItemRect, TRUE);
      }

      m_hMouseOverItem = hItem;

      CRect itemRect;
      if ((hItem != NULL) && m_renderer.GetItemRect(hItem, &itemRect))
      {
         InvalidateRect(itemRect, FALSE);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
