/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPalette.h"
#include "BitmapUtils.h"
#include "DynamicLink.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////

static const int kTextGap = 2;
static const int kImageGap = 1;
static const int kCheckedItemTextOffset = 2;

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

cToolItem::cToolItem(cToolGroup * pGroup, const tChar * pszName, int iImage, void * pUserData)
 : m_pGroup(pGroup),
   m_name(pszName != NULL ? pszName : ""),
   m_state(kTPTS_None),
   m_iImage(iImage),
   m_pUserData(pUserData)
{
}

////////////////////////////////////////

cToolItem::~cToolItem()
{
}

////////////////////////////////////////

void cToolItem::SetState(uint mask, uint state)
{
   m_state = (m_state & ~mask) | (state & mask);
}

////////////////////////////////////////

void cToolItem::ToggleChecked()
{
   SetState(kTPTS_Checked, IsChecked() ? 0 : kTPTS_Checked);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolGroup
//

////////////////////////////////////////

cToolGroup::cToolGroup(const tChar * pszName, HIMAGELIST hImageList)
 : m_name(pszName != NULL ? pszName : ""),
   m_hImageList(hImageList),
   m_bCollapsed(false)
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

HTOOLITEM cToolGroup::AddTool(const tChar * pszTool, int iImage, void * pUserData)
{
   if (pszTool != NULL)
   {
      HTOOLITEM hTool = FindTool(pszTool);
      if (hTool != NULL)
      {
         return hTool;
      }

      cToolItem * pTool = new cToolItem(this, pszTool, iImage, pUserData);
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
 : m_bHaveMousePos(false),
   m_totalHeight(0)
{
}

////////////////////////////////////////

cToolPaletteRenderer::cToolPaletteRenderer(const cToolPaletteRenderer & other)
 : m_dc(other.m_dc),
   m_rect(other.m_rect),
   m_mousePos(other.m_mousePos),
   m_bHaveMousePos(other.m_bHaveMousePos),
   m_totalHeight(other.m_totalHeight)
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
   m_totalHeight = other.m_totalHeight;
   return *this;
}

////////////////////////////////////////

bool cToolPaletteRenderer::Begin(HDC hDC, LPCRECT pRect, const POINT * pMousePos)
{
   if (m_checkedItemBrush.IsNull())
   {
      static const WORD patternBits[] =
      {
         0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,
      };

      HBITMAP hPatternBm = CreateBitmap(8, 8, 1, 1, patternBits);
      if (hPatternBm != NULL)
      {
         m_checkedItemBrush.CreatePatternBrush(hPatternBm);
         DeleteObject(hPatternBm);
      }
   }

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
      m_totalHeight = 0;
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

   CRect r(m_rect);
   r.top += m_totalHeight;

   int headingHeight = RenderGroupHeading(m_dc, &r, pGroup);

   if (headingHeight > 0)
   {
      m_cachedRects[reinterpret_cast<HANDLE>(const_cast<cToolGroup *>(pGroup))] = r;
   }

   if (pGroup->IsCollapsed())
   {
      m_totalHeight += r.Height();
      return;
   }

   HIMAGELIST hNormalImages = pGroup->GetImageList();

   CRect toolRect(m_rect);
   toolRect.top += m_totalHeight + headingHeight;

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
         CPoint imageOffset(0,0);

         if ((hNormalImages != NULL) && (iImage > -1))
         {
            IMAGEINFO imageInfo;
            if (ImageList_GetImageInfo(hNormalImages, iImage, &imageInfo))
            {
               int imageHeight = (imageInfo.rcImage.bottom - imageInfo.rcImage.top);
               int imageWidth = (imageInfo.rcImage.right - imageInfo.rcImage.left);
               if ((imageHeight + (2 * kImageGap)) > toolRect.Height())
               {
                  toolRect.bottom = toolRect.top + imageHeight + (2 * kImageGap);
               }
               imageOffset.x = kImageGap;
               imageOffset.y = ((toolRect.Height() - imageHeight) / 2);
               textRect.left = toolRect.left + imageOffset.x + imageWidth;
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

         m_dc.FillSolidRect(toolRect, GetSysColor(COLOR_3DFACE));

         if (!pTool->IsDisabled())
         {
            if (pTool->IsChecked())
            {
               m_dc.Draw3dRect(toolRect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));

               textRect.left += kCheckedItemTextOffset;
               textRect.top += kCheckedItemTextOffset;

               imageOffset.x += kCheckedItemTextOffset;
               imageOffset.y += kCheckedItemTextOffset;

               CRect tr2(toolRect);
               tr2.DeflateRect(2, 2);

               if (!m_checkedItemBrush.IsNull())
               {
                  COLORREF oldTextColor = m_dc.SetTextColor(GetSysColor(COLOR_3DHILIGHT));
                  COLORREF oldBkColor = m_dc.SetBkColor(GetSysColor(COLOR_3DFACE));
                  m_dc.FillRect(tr2, m_checkedItemBrush);
                  m_dc.SetTextColor(oldTextColor);
                  m_dc.SetBkColor(oldBkColor);
               }
            }
            else if (m_bHaveMousePos && toolRect.PtInRect(m_mousePos))
            {
               m_dc.Draw3dRect(toolRect, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
            }
         }

         if ((hNormalImages != NULL) && (iImage > -1))
         {
            // TODO: clip the image in case it is offset by being a checked item
            BOOL (STDCALL *pfnILDraw)(HIMAGELIST, int, HDC, int, int, uint) = pTool->IsDisabled()
               ? ImageList_DrawDisabled
               : ImageList_Draw;

            (*pfnILDraw)(hNormalImages, iImage, m_dc,
               toolRect.left + imageOffset.x,
               toolRect.top + imageOffset.y,
               ILD_NORMAL);
         }

         COLORREF oldTextColor = m_dc.SetTextColor(pTool->IsDisabled()
            ? GetSysColor(COLOR_GRAYTEXT)
            : GetSysColor(COLOR_WINDOWTEXT));
         int oldBkMode = m_dc.SetBkMode(TRANSPARENT);
         m_dc.DrawText(pTool->GetName(), -1, &textRect, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
         m_dc.SetTextColor(oldTextColor);
         m_dc.SetBkMode(oldBkMode);

         toolRect.OffsetRect(0, toolRect.Height());
      }
   }

   m_totalHeight = toolRect.top;

   if (m_totalHeight < m_rect.Height())
   {
      m_dc.FillSolidRect(m_rect.left, m_totalHeight, m_rect.right, m_rect.bottom, GetSysColor(COLOR_3DFACE));
   }
}

////////////////////////////////////////

int cToolPaletteRenderer::RenderGroupHeading(WTL::CDCHandle dc, LPRECT pRect,
                                             const cToolGroup * pGroup)
{
   if ((pGroup == NULL) || (lstrlen(pGroup->GetName()) == 0))
   {
      return 0;
   }

   Assert(!dc.IsNull());
   Assert(pRect != NULL);

   CSize extent;
   dc.GetTextExtent(pGroup->GetName(), -1, &extent);

   extent.cy += (2 * kTextGap);

   pRect->bottom = pRect->top + extent.cy;

   dc.Draw3dRect(pRect, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));

   CRect textRect(*pRect);
   textRect.DeflateRect(1, 1);

   int halfWidth = textRect.Width() / 2;

   COLORREF leftColor = GetSysColor(COLOR_3DSHADOW);
   COLORREF rightColor = GetSysColor(COLOR_3DFACE);

   TRIVERTEX gv[2];
   gv[0].x = textRect.left;
   gv[0].y = textRect.top;
   gv[0].Red = GetRValue16(leftColor);
   gv[0].Green = GetGValue16(leftColor);
   gv[0].Blue = GetBValue16(leftColor);
   gv[0].Alpha = 0;
   gv[1].x = textRect.left + halfWidth;
   gv[1].y = textRect.bottom; 
   gv[1].Red = GetRValue16(rightColor);
   gv[1].Green = GetGValue16(rightColor);
   gv[1].Blue = GetBValue16(rightColor);
   gv[1].Alpha = 0;

   GRADIENT_RECT gr;
   gr.UpperLeft = 0;
   gr.LowerRight = 1;
   if (DynamicGradientFill(dc, gv, _countof(gv), &gr, 1, GRADIENT_FILL_RECT_H))
   {
      dc.FillSolidRect(textRect.left + halfWidth, textRect.top, halfWidth, textRect.Height(), rightColor);
   }
   else
   {
      dc.FillSolidRect(textRect.left, textRect.top, textRect.Width(), textRect.Height(), rightColor);
   }

   textRect.left += kTextGap;

   dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
   dc.SetBkMode(TRANSPARENT);
   dc.DrawText(pGroup->GetName(), -1, &textRect, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

   return extent.cy;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cToolPaletteTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cToolPaletteTests);
      CPPUNIT_TEST(TestCallsWithBadHandles);
      CPPUNIT_TEST(TestAddRemoveGroups);
      CPPUNIT_TEST(Test1);
   CPPUNIT_TEST_SUITE_END();

   void TestCallsWithBadHandles();
   void TestAddRemoveGroups();
   void Test1();

public:
   cToolPaletteTests();
   ~cToolPaletteTests();

   virtual void setUp();
   virtual void tearDown();

private:
   typedef CWinTraits<WS_POPUP, 0> tDummyWinTraits;
   class cDummyWindow : public CWindowImpl<cDummyWindow, CWindow, tDummyWinTraits>
   {
   public:
      DECLARE_WND_CLASS(NULL);
      DECLARE_EMPTY_MSG_MAP()
   };

   cDummyWindow * m_pDummyWnd;
   cToolPalette * m_pToolPalette;
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cToolPaletteTests);

////////////////////////////////////////

void cToolPaletteTests::TestCallsWithBadHandles()
{
   if (m_pToolPalette == NULL || !m_pToolPalette->IsWindow())
   {
      return;
   }

   std::string s;
   sToolPaletteItem tpi;

   HTOOLGROUP hBadGroup = (HTOOLGROUP)0xDEADBEEF;
   HTOOLITEM hBadTool = (HTOOLITEM)0xDEADBEEF;

   CPPUNIT_ASSERT(!m_pToolPalette->RemoveGroup(hBadGroup));
   CPPUNIT_ASSERT(!m_pToolPalette->IsGroup(hBadGroup));
   CPPUNIT_ASSERT(!m_pToolPalette->IsTool(hBadTool));
   CPPUNIT_ASSERT(!m_pToolPalette->AddTool(hBadGroup, "foo", -1));
   CPPUNIT_ASSERT(!m_pToolPalette->AddTool(hBadGroup, &tpi));
   CPPUNIT_ASSERT(!m_pToolPalette->GetToolText(hBadTool, &s));
   CPPUNIT_ASSERT(!m_pToolPalette->GetTool(hBadTool, &tpi));
   CPPUNIT_ASSERT(!m_pToolPalette->RemoveTool(hBadTool));
   CPPUNIT_ASSERT(!m_pToolPalette->EnableTool(hBadTool, true));
}

////////////////////////////////////////

void cToolPaletteTests::TestAddRemoveGroups()
{
   if (m_pToolPalette == NULL || !m_pToolPalette->IsWindow())
   {
      return;
   }

   uint i, nGroupsAdd = 10 + (rand() % 90);

   std::vector<HTOOLGROUP> groups;

   for (i = 0; i < nGroupsAdd; i++)
   {
      tChar szTemp[200];
      wsprintf(szTemp, "Group %d", i);
      HTOOLGROUP hToolGroup = m_pToolPalette->AddGroup(szTemp, NULL);
      CPPUNIT_ASSERT(hToolGroup != NULL);
      groups.push_back(hToolGroup);
   }

   CPPUNIT_ASSERT(m_pToolPalette->GetGroupCount() == nGroupsAdd);

   uint nGroupsRemove = nGroupsAdd - (rand() % (nGroupsAdd / 2));
   CPPUNIT_ASSERT(nGroupsRemove < nGroupsAdd);

   for (i = 0; i < nGroupsRemove; i++)
   {
      int iRemove = rand() % groups.size();
      CPPUNIT_ASSERT(m_pToolPalette->RemoveGroup(groups[iRemove]));
      groups.erase(groups.begin() + iRemove);
   }

   CPPUNIT_ASSERT(m_pToolPalette->GetGroupCount() == (nGroupsAdd - nGroupsRemove));
}

////////////////////////////////////////

void cToolPaletteTests::Test1()
{
   if (m_pToolPalette == NULL || !m_pToolPalette->IsWindow())
   {
      return;
   }

   for (int i = 0; i < 3; i++)
   {
      tChar szTemp[200];
      wsprintf(szTemp, "Group %c", 'A' + i);
      HTOOLGROUP hToolGroup = m_pToolPalette->AddGroup(szTemp, NULL);
      if (hToolGroup != NULL)
      {
         int nTools = 3 + (rand() & 3);
         for (int j = 0; j < nTools; j++)
         {
            wsprintf(szTemp, "Tool %d", j);
            HTOOLITEM hTool = m_pToolPalette->AddTool(hToolGroup, szTemp, -1);
            if (hTool != NULL)
            {
               if (j == 1)
               {
                  CPPUNIT_ASSERT(m_pToolPalette->EnableTool(hTool, false));
               }
            }
         }
      }
   }
}

////////////////////////////////////////

cToolPaletteTests::cToolPaletteTests()
 : m_pDummyWnd(NULL),
   m_pToolPalette(NULL)
{
}

////////////////////////////////////////

cToolPaletteTests::~cToolPaletteTests()
{
}

////////////////////////////////////////

void cToolPaletteTests::setUp()
{
   CPPUNIT_ASSERT(m_pDummyWnd == NULL);
   m_pDummyWnd = new cDummyWindow;
   CPPUNIT_ASSERT(m_pDummyWnd != NULL);
   HWND hDummyWnd = m_pDummyWnd->Create(NULL, CWindow::rcDefault);
   CPPUNIT_ASSERT(IsWindow(hDummyWnd));
   if (!IsWindow(hDummyWnd))
   {
      delete m_pDummyWnd;
      m_pDummyWnd = NULL;
   }
   else
   {
      CPPUNIT_ASSERT(m_pToolPalette == NULL);
      m_pToolPalette = new cToolPalette;
      CPPUNIT_ASSERT(m_pToolPalette != NULL);
      HWND hWndToolPalette = m_pToolPalette->Create(m_pDummyWnd->m_hWnd, CWindow::rcDefault);
      CPPUNIT_ASSERT(IsWindow(hWndToolPalette));
      if (!IsWindow(hWndToolPalette))
      {
         delete m_pToolPalette;
         m_pToolPalette = NULL;
      }
   }
}

////////////////////////////////////////

void cToolPaletteTests::tearDown()
{
   if (m_pToolPalette != NULL && m_pToolPalette->IsWindow())
   {
      CPPUNIT_ASSERT(m_pToolPalette->DestroyWindow());
   }
   delete m_pToolPalette;
   m_pToolPalette = NULL;

   if (m_pDummyWnd != NULL && m_pDummyWnd->IsWindow())
   {
      CPPUNIT_ASSERT(m_pDummyWnd->DestroyWindow());
   }
   delete m_pDummyWnd;
   m_pDummyWnd = NULL;
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
