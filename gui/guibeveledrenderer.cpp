///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibeveledrenderer.h"
#include "guielementtools.h"
#include "guistrings.h"
#include "gui/guistyleapi.h"

#include "render/renderapi.h"
#include "render/renderfontapi.h"

#include "tech/configapi.h"
#include "tech/globalobj.h"
#include "tech/imageapi.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBeveledRenderer
//

static const int kDefaultBevel = 2;
static const uint kDefaultEditSize = 20;
static const uint kVertInset = 2; // TODO make this part of the style
static const uint kHorzInset = 2; // TODO make this part of the style
static const uint kCursorWidth = 1;
static const tGUISizeType kScrollButtonSize = 16;

///////////////////////////////////////

cGUIBeveledRenderer::cGUIBeveledRenderer(uint bevel, const tGUIColor colorScheme[kBC_NumColors])
 : m_bevel(bevel)
 , m_texture(NULL)
{
   if (colorScheme != NULL)
   {
      memcpy(&m_colorScheme[0], &colorScheme[0], sizeof(m_colorScheme));
   }
}

///////////////////////////////////////

cGUIBeveledRenderer::~cGUIBeveledRenderer()
{
}

///////////////////////////////////////

#define METHOD_TABLE_ENTRY(ElementType) \
   { &IID_IGUI##ElementType##Element, &cGUIBeveledRenderer::ElementType##Render, &cGUIBeveledRenderer::ElementType##PreferredSize }

const cGUIBeveledRenderer::sMethodTableEntry cGUIBeveledRenderer::gm_methodTable[] =
{
   METHOD_TABLE_ENTRY(Button),
   METHOD_TABLE_ENTRY(Label),
   METHOD_TABLE_ENTRY(ListBox),
   METHOD_TABLE_ENTRY(ScrollBar),
   METHOD_TABLE_ENTRY(TextEdit),
   METHOD_TABLE_ENTRY(TitleBar),

   // Must stay at the bottom
   METHOD_TABLE_ENTRY(Container),
};

#undef METHOD_TABLE_ENTRY

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIElement * pElement, const tGUIPoint & position, IRender2D * pRender2D)
{
   if (pRender2D == NULL || pElement == NULL)
   {
      return E_POINTER;
   }

   if (m_texture == NULL)
   {
      cAutoIPtr<IImage> pImage;
      if (ImageCreate(512, 512, kPF_RGBA8888, NULL, &pImage) == S_OK)
      {
         // TODO

         void * texture = NULL;
         UseGlobal(Renderer);
         if (pRenderer->CreateTexture(pImage, false, &texture) == S_OK)
         {
            m_texture = texture;
         }
      }
   }

   for (int i = 0; i < _countof(gm_methodTable); i++)
   {
      cAutoIPtr<IGUIElement> pElement2;
      if (pElement->QueryInterface(*(gm_methodTable[i].pIID), (void**)&pElement2) == S_OK)
      {
         if (gm_methodTable[i].pfnRender != NULL)
         {
            cAutoIPtr<IRenderFont> pFont;
            cAutoIPtr<IGUIStyle> pStyle;
            if (pElement2->GetStyle(&pStyle) == S_OK)
            {
               GUIStyleFontCreate(pStyle, &pFont);
            }
            if (!pFont)
            {
               UseGlobal(GUIContext);
               pGUIContext->GetDefaultFont(&pFont);
            }

            tGUISize size = pElement2->GetSize();
            tGUIRect rect(
               FloatToInt(position.x), FloatToInt(position.y),
               FloatToInt(position.x + size.width), FloatToInt(position.y + size.height));

            return (this->*(gm_methodTable[i].pfnRender))(pElement2, pStyle, pFont, rect, pRender2D);
         }
         else
         {
            return S_FALSE;
         }
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::GetPreferredSize(IGUIElement * pElement, const tGUISize & parentSize, tGUISize * pSize)
{
   if (pElement == NULL || pSize == NULL)
   {
      return E_POINTER;
   }

   for (int i = 0; i < _countof(gm_methodTable); i++)
   {
      cAutoIPtr<IGUIElement> pElement2;
      if (pElement->QueryInterface(*(gm_methodTable[i].pIID), (void**)&pElement2) == S_OK)
      {
         if (gm_methodTable[i].pfnPreferredSize != NULL)
         {
            cAutoIPtr<IRenderFont> pFont;
            cAutoIPtr<IGUIStyle> pStyle;
            if (pElement2->GetStyle(&pStyle) == S_OK)
            {
               GUIStyleFontCreate(pStyle, &pFont);
            }
            if (!pFont)
            {
               UseGlobal(GUIContext);
               pGUIContext->GetDefaultFont(&pFont);
            }

            *pSize = (this->*(gm_methodTable[i].pfnPreferredSize))(pElement2, pFont, parentSize);
            return S_OK;
         }
         else
         {
            return S_FALSE;
         }
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::AllocateBorderSpace(IGUIElement * pElement, tGUIRect * pRect)
{
   if (pRect == NULL)
   {
      return E_POINTER;
   }
   pRect->left += m_bevel;
   pRect->top += m_bevel;
   pRect->right -= m_bevel;
   pRect->bottom -= m_bevel;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::ButtonRender(IGUIElement * pElement,
                                          IGUIStyle * pStyle,
                                          IRenderFont * pFont,
                                          const tGUIRect & rect,
                                          IRender2D * pRender2D)
{
   bool bPressed = false;

   IGUIButtonElement * pButtonElement = (IGUIButtonElement *)pElement;

   if (pButtonElement->IsArmed() && pButtonElement->IsMouseOver())
   {
      pRender2D->RenderBeveledRect(rect, GetBevel(), GetColor(kBC_Shadow).GetPointer(), GetColor(kBC_Highlight).GetPointer(), GetColor(kBC_Face).GetPointer());
      bPressed = true;
   }
   else
   {
      pRender2D->RenderBeveledRect(rect, GetBevel(), GetColor(kBC_Highlight).GetPointer(), GetColor(kBC_Shadow).GetPointer(), GetColor(kBC_Face).GetPointer());
   }

   const tGUIChar * pszText = pButtonElement->GetText();
   if (pszText != NULL)
   {
      if (pFont != NULL)
      {
         uint renderTextFlags = kRT_Center | kRT_VCenter | kRT_SingleLine;

         cAutoIPtr<IGUIStyle> pStyle;
         if (pElement->GetStyle(&pStyle) == S_OK)
         {
            int dropShadow = 0;
            if (pStyle->GetAttribute(kAttribDropShadow, &dropShadow) == S_OK
               && dropShadow != 0)
            {
               renderTextFlags |= kRT_DropShadow;
            }
         }

         tGUIRect textRect(rect);

         if (bPressed)
         {
            textRect.left += GetBevel();
            textRect.top += GetBevel();
         }

         pFont->RenderText(pszText, -1, &textRect, renderTextFlags,
            pElement->IsEnabled() ? GUIStandardColors::White.GetPointer() : GetColor(kBC_Shadow).GetPointer());

         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::ButtonPreferredSize(IGUIElement * pElement,
                                                  IRenderFont * pFont,
                                                  const tGUISize & parentSize) const
{
   IGUIButtonElement * pButtonElement = (IGUIButtonElement *)pElement;

   const tGUIChar * pszText = pButtonElement->GetText();
   if (pszText != NULL)
   {
      if (pFont != NULL)
      {
         tRect rect(0,0,0,0);
         pFont->RenderText(pszText, -1, &rect, kRT_CalcRect, NULL);
         return tGUISize(static_cast<tGUISizeType>(rect.GetWidth() + rect.GetHeight()), rect.GetHeight() * 1.5f);
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::LabelRender(IGUIElement * pElement,
                                         IGUIStyle * pStyle,
                                         IRenderFont * pFont,
                                         const tGUIRect & rect,
                                         IRender2D * pRender2D)
{
   IGUILabelElement * pLabelElement = (IGUILabelElement *)pElement;

   tGUIColor color(GUIStandardColors::Black);

   if (pStyle != NULL)
   {
      pStyle->GetForegroundColor(&color);
   }

   const tGUIChar * pszText = pLabelElement->GetText();
   if (pszText != NULL)
   {
      if (pFont != NULL)
      {
         tGUIRect textRect(rect);
         pFont->RenderText(pszText, -1, &textRect, kRT_NoClip, color.GetPointer());
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::LabelPreferredSize(IGUIElement * pElement,
                                                 IRenderFont * pFont,
                                                 const tGUISize & parentSize) const
{
   IGUILabelElement * pLabelElement = (IGUILabelElement *)pElement;

   const tGUIChar * pszText = pLabelElement->GetText();
   if (pszText != NULL)
   {
      if (pFont != NULL)
      {
         tRect rect(0,0,0,0);
         pFont->RenderText(pszText, -1, &rect, kRT_CalcRect, NULL);
         return tGUISize(static_cast<tGUISizeType>(rect.GetWidth()), static_cast<tGUISizeType>(rect.GetHeight()));
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::ListBoxRender(IGUIElement * pElement,
                                           IGUIStyle * pStyle,
                                           IRenderFont * pFont,
                                           const tGUIRect & rect,
                                           IRender2D * pRender2D)
{
   if (rect.GetWidth() == 0 || rect.GetHeight() == 0)
   {
      return S_FALSE;
   }

   IGUIListBoxElement * pListBoxElement = (IGUIListBoxElement *)pElement;

   pRender2D->RenderBeveledRect(rect, GetBevel(), GUIStandardColors::DarkGray.GetPointer(), GUIStandardColors::Gray.GetPointer(), GUIStandardColors::White.GetPointer());

   tGUIRect insetRect(rect);
   insetRect.left += GetBevel() + kHorzInset;
   insetRect.top += kVertInset;
   insetRect.right -= GetBevel() + kHorzInset;
   insetRect.bottom -= kVertInset;

   pRender2D->PushScissorRect(insetRect);

   tGUIColor textColor(GUIStandardColors::Black);
   if (pStyle != NULL)
   {
      pStyle->GetForegroundColor(&textColor);
   }

   if (pFont != NULL)
   {
      int scrollPos = 0;
      cAutoIPtr<IGUIScrollBarElement> pVertScrollBar;
      if (pListBoxElement->GetVerticalScrollBar(&pVertScrollBar) == S_OK)
      {
         if (pVertScrollBar->GetScrollPos(&scrollPos) != S_OK)
         {
            scrollPos = 0;
         }
      }

      uint itemCount = 0;
      if (pListBoxElement->GetItemCount(&itemCount) == S_OK)
      {
         tGUIRect itemRect(insetRect);

         uint itemHeight = 0;
         if (pListBoxElement->GetItemHeight(&itemHeight) != S_OK)
         {
            itemHeight = ListBoxPreferredItemHeight(pFont);
            pListBoxElement->SetItemHeight(itemHeight);
         }

         uint iTopItem = static_cast<uint>(scrollPos) / itemHeight;
         uint scrollTopOffset = static_cast<uint>(scrollPos) % itemHeight;
         itemRect.top -= scrollTopOffset;

         itemRect.bottom = itemRect.top + itemHeight;

         for (uint i = iTopItem; i < itemCount; i++)
         {
            const tChar * pszText = pListBoxElement->GetItemText(i);
            if (pszText == NULL)
            {
               continue;
            }

            if (pListBoxElement->IsItemSelected(i))
            {
               pRender2D->RenderSolidRect(itemRect, GUIStandardColors::Blue.GetPointer());
               pFont->RenderText(pszText, -1, &itemRect, kRT_NoClip, GUIStandardColors::White.GetPointer());
            }
            else
            {
               pFont->RenderText(pszText, -1, &itemRect, kRT_NoClip, textColor.GetPointer());
            }

            itemRect.Offset(0, itemHeight);
         }
      }
   }

   pRender2D->PopScissorRect();

   return S_OK;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::ListBoxPreferredSize(IGUIElement * pElement,
                                                   IRenderFont * pFont,
                                                   const tGUISize & parentSize) const
{
   IGUIListBoxElement * pListBoxElement = (IGUIListBoxElement *)pElement;

   if (pFont != NULL)
   {
      uint rowCount;
      if (pListBoxElement->GetRowCount(&rowCount) == S_OK)
      {
         return tGUISize(0, static_cast<tGUISizeType>(rowCount * ListBoxPreferredItemHeight(pFont)));
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

uint cGUIBeveledRenderer::ListBoxPreferredItemHeight(IRenderFont * pFont) const
{
   tRect rect(0,0,0,0);
   pFont->RenderText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\0", -1, &rect, kRT_CalcRect, NULL);
   return rect.GetHeight();
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::ScrollBarRender(IGUIElement * pElement,
                                             IGUIStyle * pStyle,
                                             IRenderFont * pFont,
                                             const tGUIRect & rect,
                                             IRender2D * pRender2D)
{
   IGUIScrollBarElement * pScrollBarElement = (IGUIScrollBarElement *)pElement;

   int rangeMin, rangeMax, scrollPos;
   Verify(pScrollBarElement->GetRange(&rangeMin, &rangeMax) == S_OK);
   Verify(pScrollBarElement->GetScrollPos(&scrollPos) == S_OK);
   Assert(scrollPos >= rangeMin && scrollPos <= rangeMax);

   eGUIScrollBarType scrollBarType = pScrollBarElement->GetScrollBarType();

   eGUIScrollBarPart armedPart = pScrollBarElement->GetArmedPart();
   eGUIScrollBarPart mouseOverPart = pScrollBarElement->GetMouseOverPart();

   tGUIRect btn1Rect, btn2Rect, thumbRect;
   if (pScrollBarElement->GetPartRect(kGUIScrollBarPartButton1, &btn1Rect) != S_OK
      || pScrollBarElement->GetPartRect(kGUIScrollBarPartButton2, &btn2Rect) != S_OK
      || pScrollBarElement->GetPartRect(kGUIScrollBarPartThumb, &thumbRect) != S_OK)
   {
      return E_FAIL;
   }

   int offsetx = rect.left, offsety = rect.top;
   btn1Rect.Offset(offsetx, offsety);
   btn2Rect.Offset(offsetx, offsety);
   thumbRect.Offset(offsetx, offsety);

   tGUIRect track1Rect, track2Rect;
   if (scrollBarType == kGUIScrollBarHorizontal)
   {
      track1Rect = tGUIRect(btn1Rect.right, btn1Rect.top, thumbRect.left, btn1Rect.bottom);
      track2Rect = tGUIRect(thumbRect.right, btn2Rect.top, btn2Rect.left, btn2Rect.bottom);
   }
   else if (scrollBarType == kGUIScrollBarVertical)
   {
      track1Rect = tGUIRect(btn1Rect.left, btn1Rect.bottom, btn1Rect.right, thumbRect.top);
      track2Rect = tGUIRect(btn1Rect.left, thumbRect.bottom, btn1Rect.right, btn2Rect.top);
   }

   pRender2D->RenderSolidRect(track1Rect, GetColor(kBC_Highlight).GetPointer());
   pRender2D->RenderSolidRect(track2Rect, GetColor(kBC_Highlight).GetPointer());
   pRender2D->RenderBeveledRect(thumbRect, GetBevel(), GetColor(kBC_Highlight).GetPointer(), GUIStandardColors::DarkGray.GetPointer(), GetColor(kBC_Face).GetPointer());

   if (armedPart == kGUIScrollBarPartButton1 && armedPart == mouseOverPart)
   {
      pRender2D->RenderBeveledRect(btn1Rect, GetBevel(), GUIStandardColors::DarkGray.GetPointer(), GetColor(kBC_Highlight).GetPointer(), GetColor(kBC_Face).GetPointer());
   }
   else
   {
      pRender2D->RenderBeveledRect(btn1Rect, GetBevel(), GetColor(kBC_Highlight).GetPointer(), GUIStandardColors::DarkGray.GetPointer(), GetColor(kBC_Face).GetPointer());
   }

   if (armedPart == kGUIScrollBarPartButton2 && armedPart == mouseOverPart)
   {
      pRender2D->RenderBeveledRect(btn2Rect, GetBevel(), GetColor(kBC_Shadow).GetPointer(), GetColor(kBC_Highlight).GetPointer(), GetColor(kBC_Face).GetPointer());
   }
   else
   {
      pRender2D->RenderBeveledRect(btn2Rect, GetBevel(), GetColor(kBC_Highlight).GetPointer(), GetColor(kBC_Shadow).GetPointer(), GetColor(kBC_Face).GetPointer());
   }

   return S_OK;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::ScrollBarPreferredSize(IGUIElement * pElement,
                                                     IRenderFont * pFont,
                                                     const tGUISize & parentSize) const
{
   IGUIScrollBarElement * pScrollBarElement = (IGUIScrollBarElement *)pElement;

   eGUIScrollBarType scrollBarType = pScrollBarElement->GetScrollBarType();

   if (scrollBarType == kGUIScrollBarHorizontal)
   {
      return tGUISize(0, kScrollButtonSize);
   }
   else if (scrollBarType == kGUIScrollBarVertical)
   {
      return tGUISize(kScrollButtonSize, 0);
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::TextEditRender(IGUIElement * pElement,
                                            IGUIStyle * pStyle,
                                            IRenderFont * pFont,
                                            const tGUIRect & rect,
                                            IRender2D * pRender2D)
{
   IGUITextEditElement * pTextEditElement = (IGUITextEditElement *)pElement;

   pRender2D->RenderBeveledRect(rect, GetBevel(), GetColor(kBC_Shadow).GetPointer(), GetColor(kBC_Face).GetPointer(), GUIStandardColors::White.GetPointer());

   tGUIRect insetRect(rect);
   insetRect.left += GetBevel() + kHorzInset;
   insetRect.top += kVertInset;
   insetRect.right -= GetBevel() + kHorzInset;
   insetRect.bottom -= kVertInset;

   pRender2D->PushScissorRect(insetRect);

   tGUIColor textColor(GUIStandardColors::Black);

   if (pStyle != NULL)
   {
      pStyle->GetForegroundColor(&textColor);
   }

   uint selStart, selEnd;
   Verify(pTextEditElement->GetSelection(&selStart, &selEnd) == S_OK);

   const tGUIChar * pszText = pTextEditElement->GetText();
   if (pszText != NULL)
   {
      if (pFont != NULL)
      {
         pFont->RenderText(pszText, -1, &insetRect, kRT_NoClip, textColor.GetPointer());

         // Determine the width of the text up to the cursor
         tRect leftOfCursor(0,0,0,0);
         pFont->RenderText(pszText, selEnd, &leftOfCursor, kRT_NoClip | kRT_CalcRect, NULL);

         // Offset the left edge so that the cursor is always in view.
         if (leftOfCursor.GetWidth() >= insetRect.GetWidth())
         {
            insetRect.left -= leftOfCursor.GetWidth() - insetRect.GetWidth() + kCursorWidth;
         }

         // Render the cursor if this widget has focus and its blink cycle is on
         if (pTextEditElement->HasFocus() && pTextEditElement->ShowBlinkingCursor())
         {
            tGUIRect cursorRect(
               insetRect.left + leftOfCursor.GetWidth(),
               insetRect.top + 1,
               insetRect.left + leftOfCursor.GetWidth() + kCursorWidth,
               insetRect.bottom - 1);

            pRender2D->RenderSolidRect(cursorRect, GUIStandardColors::Black.GetPointer());
         }
      }
   }

   pRender2D->PopScissorRect();

   pTextEditElement->UpdateBlinkingCursor();

   return S_OK;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::TextEditPreferredSize(IGUIElement * pElement,
                                                    IRenderFont * pFont,
                                                    const tGUISize & parentSize) const
{
   IGUITextEditElement * pTextEditElement = (IGUITextEditElement *)pElement;

   uint editSize = kDefaultEditSize;
   if (pTextEditElement->GetEditSize(&editSize) != S_OK)
   {
      editSize = kDefaultEditSize;
   }

   if (pFont != NULL)
   {
      char * psz = reinterpret_cast<char *>(alloca(editSize * sizeof(char)));
      memset(psz, 'M', editSize * sizeof(char));

      tRect rect(0,0,0,0);
      pFont->RenderText(psz, editSize, &rect, kRT_CalcRect | kRT_SingleLine, NULL);

      return tGUISize(static_cast<tGUISizeType>(rect.GetWidth() + (kHorzInset * 2)),
                      static_cast<tGUISizeType>(rect.GetHeight() + (kVertInset * 2)));
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::TitleBarRender(IGUIElement * pElement,
                                            IGUIStyle * pStyle,
                                            IRenderFont * pFont,
                                            const tGUIRect & rect,
                                            IRender2D * pRender2D)
{
   IGUITitleBarElement * pTitleBarElement = (IGUITitleBarElement *)pElement;

   tGUIColor captionBk(GUIStandardColors::Blue);
   tGUIColor captionText(GUIStandardColors::White);

   if (pStyle != NULL)
   {
      pStyle->GetAttribute("caption-bk-color", &captionBk);
      pStyle->GetAttribute("caption-text-color", &captionText);
   }

   pRender2D->RenderSolidRect(rect, captionBk.GetPointer());

   if (pFont != NULL)
   {
      tGUIString title;
      if (pTitleBarElement->GetTitle(&title) == S_OK)
      {
         tGUIRect textRect(rect);
         pFont->RenderText(title.c_str(), -1, &textRect, 0, captionText.GetPointer());
      }
   }

   return S_OK;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::TitleBarPreferredSize(IGUIElement * pElement,
                                                    IRenderFont * pFont,
                                                    const tGUISize & parentSize) const
{
   IGUITitleBarElement * pTitleBarElement = (IGUITitleBarElement *)pElement;

   if (pFont != NULL)
   {
      tGUIString title;
      if (pTitleBarElement->GetTitle(&title) == S_OK)
      {
         tGUISize size(0,0);

         tRect titleSize(0,0,0,0);
         if (pFont->RenderText(title.c_str(), title.length(), &titleSize, kRT_CalcRect, NULL) == S_OK)
         {
            size.height = static_cast<tGUISizeType>(titleSize.GetHeight());
         }

         size.width = parentSize.width;

         return size;
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::ContainerRender(IGUIElement * pElement,
                                             IGUIStyle * pStyle,
                                             IRenderFont * pFont,
                                             const tGUIRect & rect,
                                             IRender2D * pRender2D)
{
   pRender2D->RenderBeveledRect(rect, GetBevel(), GetColor(kBC_Highlight).GetPointer(), GetColor(kBC_Shadow).GetPointer(), GetColor(kBC_Face).GetPointer());
   return S_OK;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::ContainerPreferredSize(IGUIElement * pElement, IRenderFont * pFont, const tGUISize & parentSize) const
{
   cAutoIPtr<IGUILayoutManager> pLayout;
   if (((IGUIContainerElement*)pElement)->GetLayout(&pLayout) == S_OK)
   {
      tGUISize size;
      if (pLayout->GetPreferredSize(pElement, &size) == S_OK)
      {
         return size;
      }
   }
   return tGUISize(0,0);
}


///////////////////////////////////////////////////////////////////////////////

tResult GUIBeveledRendererCreate(void * pReserved, IGUIElementRenderer * * ppRenderer)
{
   static cAutoIPtr<IGUIElementRenderer> pSingleInstance;

   // Since the beveled renderer is stateless, a single instance can serve all GUI elements
   if (!pSingleInstance)
   {
      int bevel = kDefaultBevel;
      ConfigGet(_T("beveled-bevel"), &bevel);

      if (bevel < 0)
      {
         bevel = 0;
      }
      else if (bevel > 10)
      {
         bevel = 10;
      }

      tGUIColor colorScheme[kBC_NumColors];
      colorScheme[kBC_Shadow] = GUIStandardColors::DarkGray;
      colorScheme[kBC_Highlight] = GUIStandardColors::LightGray;
      colorScheme[kBC_Face] = GUIStandardColors::Gray;
      colorScheme[kBC_Text] = GUIStandardColors::Black;

      cStr temp;
      if (ConfigGet(_T("beveled-shadow"), &temp) == S_OK)
      {
         GUIParseColor(temp.c_str(), &colorScheme[kBC_Shadow]);
      }
      if (ConfigGet(_T("beveled-highlight"), &temp) == S_OK)
      {
         GUIParseColor(temp.c_str(), &colorScheme[kBC_Highlight]);
      }
      if (ConfigGet(_T("beveled-face"), &temp) == S_OK)
      {
         GUIParseColor(temp.c_str(), &colorScheme[kBC_Face]);
      }
      if (ConfigGet(_T("beveled-text"), &temp) == S_OK)
      {
         GUIParseColor(temp.c_str(), &colorScheme[kBC_Text]);
      }

      pSingleInstance = static_cast<IGUIElementRenderer*>(new cGUIBeveledRenderer(bevel, colorScheme));
      if (!pSingleInstance)
      {
         return E_OUTOFMEMORY;
      }
   }

   return pSingleInstance.GetPointer(ppRenderer);
}

AUTOREGISTER_GUIRENDERERFACTORYFN(beveled, GUIBeveledRendererCreate);

///////////////////////////////////////////////////////////////////////////////
