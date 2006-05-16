///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibeveledrenderer.h"
#include "guielementtools.h"
#include "guistrings.h"
#include "guistyleapi.h"

#include "globalobj.h"

#include "dbgalloc.h" // must be last header

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

cGUIBeveledRenderer::cGUIBeveledRenderer()
{
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
   METHOD_TABLE_ENTRY(ListBox),
   { &IID_IGUIContainerElement, NULL, &cGUIBeveledRenderer::ContainerPreferredSize }, // Must be at the bottom
};

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice)
{
   if (pRenderDevice == NULL || pElement == NULL)
   {
      return E_POINTER;
   }

   static const tChar *colorAttribNames[kBC_NumColors] =
   {
      _T("beveled-shadow"),   // kBC_Shadow,
      _T("beveled-light"),    // kBC_Light,
      _T("beveled-face"),     // kBC_Face,
      _T("beveled-text"),     // kBC_Text,
   };

   tGUIColor colors[kBC_NumColors] =
   {
      GUIStandardColors::DarkGray,
      GUIStandardColors::LightGray,
      GUIStandardColors::Gray,
      GUIStandardColors::Black,
   };

   int bevel = kDefaultBevel;

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      for (int i = 0; i < kBC_NumColors; i++)
      {
         pStyle->GetAttribute(colorAttribNames[i], &colors[i]);
      }

      pStyle->GetAttribute(_T("beveled-bevel"), &bevel);
   }

   for (int i = 0; i < _countof(gm_methodTable); i++)
   {
      cAutoIPtr<IGUIElement> pElement2;
      if (pElement->QueryInterface(*(gm_methodTable[i].pIID), (void**)&pElement2) == S_OK)
      {
         if (gm_methodTable[i].pfnRender != NULL)
         {
            return (this->*(gm_methodTable[i].pfnRender))(pElement2, bevel, colors, pRenderDevice);
         }
         else
         {
            break;
         }
      }
   }

   {
      cAutoIPtr<IGUIDialogElement> pDialogElement;
      if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialogElement) == S_OK)
      {
         return Render(pDialogElement, bevel, colors, pRenderDevice);
      }
   }

   {
      cAutoIPtr<IGUILabelElement> pLabelElement;
      if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabelElement) == S_OK)
      {
         return Render(pLabelElement, bevel, colors, pRenderDevice);
      }
   }

   {
      cAutoIPtr<IGUIPanelElement> pPanelElement;
      if (pElement->QueryInterface(IID_IGUIPanelElement, (void**)&pPanelElement) == S_OK)
      {
         return Render(pPanelElement, bevel, colors, pRenderDevice);
      }
   }

   {
      cAutoIPtr<IGUITextEditElement> pTextEditElement;
      if (pElement->QueryInterface(IID_IGUITextEditElement, (void**)&pTextEditElement) == S_OK)
      {
         return Render(pTextEditElement, bevel, colors, pRenderDevice);
      }
   }

   {
      cAutoIPtr<IGUIScrollBarElement> pScrollBarElement;
      if (pElement->QueryInterface(IID_IGUIScrollBarElement, (void**)&pScrollBarElement) == S_OK)
      {
         return Render(pScrollBarElement, bevel, colors, pRenderDevice);
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      {
         cAutoIPtr<IGUIButtonElement> pButtonElement;
         if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButtonElement) == S_OK)
         {
            return GetPreferredSize(pButtonElement);
         }
      }

      {
         cAutoIPtr<IGUIDialogElement> pDialogElement;
         if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialogElement) == S_OK)
         {
            return GetPreferredSize(pDialogElement);
         }
      }

      {
         cAutoIPtr<IGUILabelElement> pLabelElement;
         if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabelElement) == S_OK)
         {
            return GetPreferredSize(pLabelElement);
         }
      }

      {
         cAutoIPtr<IGUIPanelElement> pPanelElement;
         if (pElement->QueryInterface(IID_IGUIPanelElement, (void**)&pPanelElement) == S_OK)
         {
            return GetPreferredSize(pPanelElement);
         }
      }

      {
         cAutoIPtr<IGUITextEditElement> pTextEditElement;
         if (pElement->QueryInterface(IID_IGUITextEditElement, (void**)&pTextEditElement) == S_OK)
         {
            return GetPreferredSize(pTextEditElement);
         }
      }

      {
         cAutoIPtr<IGUIScrollBarElement> pScrollBarElement;
         if (pElement->QueryInterface(IID_IGUIScrollBarElement, (void**)&pScrollBarElement) == S_OK)
         {
            return GetPreferredSize(pScrollBarElement);
         }
      }

      {
         cAutoIPtr<IGUIListBoxElement> pListBoxElement;
         if (pElement->QueryInterface(IID_IGUIListBoxElement, (void**)&pListBoxElement) == S_OK)
         {
            return GetPreferredSize(pListBoxElement);
         }
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::GetPreferredSize(IGUIElement * pElement, tGUISize * pSize)
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
         *pSize = (this->*(gm_methodTable[i].pfnPreferredSize))(pElement2);
         return S_OK;
      }
   }

   *pSize = GetPreferredSize(pElement);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::ComputeClientArea(IGUIElement * pElement, tGUIRect * pClientArea)
{
   if (pElement == NULL || pClientArea == NULL)
   {
      return E_POINTER;
   }

   int bevel = kDefaultBevel;
   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetAttribute(_T("beveled-bevel"), &bevel);
   }

   tGUISize size = pElement->GetSize();
   tGUIRect clientArea(bevel, bevel, FloatToInt(size.width - bevel), FloatToInt(size.height - bevel));

   {
      cAutoIPtr<IGUIDialogElement> pDialogElement;
      if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialogElement) == S_OK)
      {
         uint captionHeight = 0;
         if (pDialogElement->GetCaptionHeight(&captionHeight) != S_OK)
         {
            cAutoIPtr<IGUIFont> pFont;
            if (GetFont(pDialogElement, &pFont) == S_OK)
            {
               tGUIString title;
               if (pDialogElement->GetTitle(&title) == S_OK)
               {
                  tRect rect(0,0,0,0);
                  if (pFont->RenderText(title.c_str(), title.length(), &rect, kRT_CalcRect, GUIStandardColors::White) == S_OK)
                  {
                     captionHeight = rect.GetHeight();
                     pDialogElement->SetCaptionHeight(captionHeight);
                  }
               }
            }
         }

         clientArea.top += captionHeight;
      }
   }

   *pClientArea = clientArea;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::GetFont(IGUIElement * pElement,
                                     IGUIFont * * ppFont) const
{
   return GUIElementFont(pElement, ppFont);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::ButtonRender(IGUIElement * pElement, int bevel,
                                          const tGUIColor colors[kBC_NumColors],
                                          IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pElement);
   tGUISize size = pElement->GetSize();

   bool bPressed = false;

   tGUIRect rect(FloatToInt(pos.x), FloatToInt(pos.y), FloatToInt(pos.x + size.width), FloatToInt(pos.y + size.height));

   IGUIButtonElement * pButtonElement = (IGUIButtonElement *)pElement;

   if (pButtonElement->IsArmed() && pButtonElement->IsMouseOver())
   {
      pRenderDevice->RenderBeveledRect(rect, bevel, colors[kBC_Shadow], colors[kBC_Light], colors[kBC_Face]);
      bPressed = true;
   }
   else
   {
      pRenderDevice->RenderBeveledRect(rect, bevel, colors[kBC_Light], colors[kBC_Shadow], colors[kBC_Face]);
   }

   tGUIString text;
   cAutoIPtr<IGUIFont> pFont;
   if (pButtonElement->GetText(&text) == S_OK
      && GetFont(pElement, &pFont) == S_OK)
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

      if (bPressed)
      {
         rect.left += bevel;
         rect.top += bevel;
      }

      pFont->RenderText(text.c_str(), text.length(), &rect, renderTextFlags,
         pElement->IsEnabled() ? GUIStandardColors::White : colors[kBC_Shadow]);

      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::ButtonPreferredSize(IGUIElement * pElement) const
{
   tGUIString text;
   cAutoIPtr<IGUIFont> pFont;
   if (((IGUIButtonElement*)pElement)->GetText(&text) == S_OK
      && GetFont(pElement, &pFont) == S_OK)
   {
      tRect rect(0,0,0,0);
      pFont->RenderText(text.c_str(), text.length(), &rect, kRT_CalcRect, GUIStandardColors::White);

      return tGUISize(static_cast<tGUISizeType>(rect.GetWidth() + rect.GetHeight()),
                      rect.GetHeight() * 1.5f);
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::ListBoxRender(IGUIElement * pElement, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice)
{
   IGUIListBoxElement * pListBoxElement = (IGUIListBoxElement *)pElement;

   tGUIPoint pos = GUIElementAbsolutePosition(pListBoxElement);
   tGUISize size = pListBoxElement->GetSize();
   tGUIRect rect(FloatToInt(pos.x), FloatToInt(pos.y), FloatToInt(pos.x + size.width), FloatToInt(pos.y + size.height));

   pRenderDevice->RenderBeveledRect(rect, bevel, GUIStandardColors::DarkGray, GUIStandardColors::Gray, GUIStandardColors::White);

   rect.left += bevel + kHorzInset;
   rect.top += kVertInset;
   rect.right -= bevel + kHorzInset;
   rect.bottom -= kVertInset;

   pRenderDevice->PushScissorRect(rect);

   tGUIColor textColor(GUIStandardColors::Black);
   cAutoIPtr<IGUIStyle> pStyle;
   if (pListBoxElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetForegroundColor(&textColor);
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pListBoxElement, &pFont) == S_OK)
   {
      uint itemCount = 0;
      if (pListBoxElement->GetItemCount(&itemCount) == S_OK)
      {
         uint itemHeight = 0;
         pListBoxElement->GetItemHeight(&itemHeight);

         tGUIRect itemRect(rect);
         itemRect.bottom = itemRect.top + itemHeight;

         for (uint i = 0; i < itemCount; i++)
         {
            cStr itemString;
            bool bIsSelected = false;
            if (FAILED(pListBoxElement->GetItem(i, &itemString, NULL, &bIsSelected)))
            {
               continue;
            }

            if (itemHeight == 0)
            {
               itemRect = rect;
               pFont->RenderText(itemString.c_str(), itemString.length(), &itemRect, kRT_CalcRect, textColor);
               itemHeight = itemRect.GetHeight();
               pListBoxElement->SetItemHeight(itemHeight);
            }

            if (bIsSelected)
            {
               pRenderDevice->RenderSolidRect(itemRect, GUIStandardColors::Blue);
               pFont->RenderText(itemString.c_str(), itemString.length(), &itemRect, kRT_NoClip, GUIStandardColors::White);
            }
            else
            {
               pFont->RenderText(itemString.c_str(), itemString.length(), &itemRect, kRT_NoClip, textColor);
            }

            itemRect.Offset(0, itemHeight);
         }
      }
   }

   pRenderDevice->PopScissorRect();

   return S_OK;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::ListBoxPreferredSize(IGUIElement * pElement) const
{
   IGUIListBoxElement * pListBoxElement = (IGUIListBoxElement *)pElement;

   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pListBoxElement, &pFont) == S_OK)
   {
      uint rowCount;
      if (pListBoxElement->GetRowCount(&rowCount) == S_OK)
      {
         tRect rect(0,0,0,0);
         pFont->RenderText("XYZxyz\0", -1, &rect, kRT_CalcRect, GUIStandardColors::White);
         return tGUISize(0, static_cast<tGUISizeType>(rowCount * rect.GetHeight()));
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::ContainerPreferredSize(IGUIElement * pElement) const
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

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIDialogElement * pDialogElement,
                                    int bevel, const tGUIColor colors[kBC_NumColors],
                                    IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pDialogElement);
   tGUISize size = pDialogElement->GetSize();
   tGUIRect rect(FloatToInt(pos.x), FloatToInt(pos.y), FloatToInt(pos.x + size.width), FloatToInt(pos.y + size.height));

   tGUIColor caption(GUIStandardColors::Blue);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pDialogElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetAttribute("caption-color", &caption);
   }

   pRenderDevice->RenderBeveledRect(rect, bevel, colors[kBC_Light], colors[kBC_Shadow], colors[kBC_Face]);

   uint captionHeight;
   if ((pDialogElement->GetCaptionHeight(&captionHeight) == S_OK)
      && (captionHeight > 0))
   {
      tGUIRect captionRect(rect);

      captionRect.left += bevel;
      captionRect.top += bevel;
      captionRect.right -= bevel;
      captionRect.bottom = captionRect.top + captionHeight;

      pRenderDevice->RenderSolidRect(captionRect, caption);

      cAutoIPtr<IGUIFont> pFont;
      if (GetFont(pDialogElement, &pFont) == S_OK)
      {
         tGUIString title;
         if (pDialogElement->GetTitle(&title) == S_OK)
         {
            pFont->RenderText(title.c_str(), -1, &captionRect, 0, GUIStandardColors::White);
         }
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUILabelElement * pLabelElement,
                                    int bevel, const tGUIColor colors[kBC_NumColors],
                                    IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pLabelElement);
   tGUISize size = pLabelElement->GetSize();
   tGUIRect rect(FloatToInt(pos.x), FloatToInt(pos.y), FloatToInt(pos.x + size.width), FloatToInt(pos.y + size.height));

   tGUIColor color(GUIStandardColors::Black);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pLabelElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetForegroundColor(&color);
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pLabelElement, &pFont) == S_OK)
   {
      tGUIString text;
      if (pLabelElement->GetText(&text) == S_OK)
      {
         pFont->RenderText(text.c_str(), text.length(), &rect, kRT_NoClip, color);
         return S_OK;
      }
      if (text.empty())
      {
         return S_FALSE;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIPanelElement * pPanelElement,
                                    int bevel, const tGUIColor colors[kBC_NumColors],
                                    IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pPanelElement);
   tGUISize size = pPanelElement->GetSize();
   tGUIRect rect(FloatToInt(pos.x), FloatToInt(pos.y), FloatToInt(pos.x + size.width), FloatToInt(pos.y + size.height));
   pRenderDevice->RenderBeveledRect(rect, bevel, colors[kBC_Light], colors[kBC_Shadow], colors[kBC_Face]);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUITextEditElement * pTextEditElement,
                                    int bevel, const tGUIColor colors[kBC_NumColors],
                                    IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pTextEditElement);
   tGUISize size = pTextEditElement->GetSize();
   tGUIRect rect(FloatToInt(pos.x), FloatToInt(pos.y), FloatToInt(pos.x + size.width), FloatToInt(pos.y + size.height));

   pRenderDevice->RenderBeveledRect(rect, bevel, colors[kBC_Shadow], colors[kBC_Face], GUIStandardColors::White);

   rect.left += bevel + kHorzInset;
   rect.top += kVertInset;
   rect.right -= bevel + kHorzInset;
   rect.bottom -= kVertInset;

   pRenderDevice->PushScissorRect(rect);

   tGUIColor textColor(GUIStandardColors::Black);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pTextEditElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetForegroundColor(&textColor);
   }

   uint selStart, selEnd;
   Verify(pTextEditElement->GetSelection(&selStart, &selEnd) == S_OK);

   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pTextEditElement, &pFont) == S_OK)
   {
      tRect leftOfCursor(0,0,0,0);

      tGUIString text;
      if (pTextEditElement->GetText(&text) == S_OK)
      {
         pFont->RenderText(text.c_str(), text.length(), &rect, kRT_NoClip, textColor);

         // Determine the width of the text up to the cursor
         pFont->RenderText(text.c_str(), selEnd, &leftOfCursor,
            kRT_NoClip | kRT_CalcRect, GUIStandardColors::White);
      }

      // Offset the left edge so that the cursor is always in view.
      if (leftOfCursor.GetWidth() >= rect.GetWidth())
      {
         rect.left -= leftOfCursor.GetWidth() - rect.GetWidth() + kCursorWidth;
      }

      // Render the cursor if this widget has focus and its blink cycle is on
      if (pTextEditElement->HasFocus() && pTextEditElement->ShowBlinkingCursor())
      {
         tGUIRect cursorRect(
            rect.left + leftOfCursor.GetWidth(),
            rect.top + 1,
            rect.left + leftOfCursor.GetWidth() + kCursorWidth,
            rect.bottom - 1);

         pRenderDevice->RenderSolidRect(cursorRect, GUIStandardColors::Black);
      }
   }

   pRenderDevice->PopScissorRect();

   pTextEditElement->UpdateBlinkingCursor();

   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIScrollBarElement * pScrollBarElement,
                                    int bevel, const tGUIColor colors[kBC_NumColors],
                                    IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pScrollBarElement);
   tGUISize size = pScrollBarElement->GetSize();
   tGUIRect rect(FloatToInt(pos.x), FloatToInt(pos.y), FloatToInt(pos.x + size.width), FloatToInt(pos.y + size.height));

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

   int offsetx = FloatToInt(pos.x), offsety = FloatToInt(pos.y);
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
      track2Rect = tGUIRect(thumbRect.right, btn2Rect.top, btn2Rect.left, btn2Rect.bottom);
   }

   pRenderDevice->RenderSolidRect(track1Rect, colors[kBC_Light]);
   pRenderDevice->RenderSolidRect(track2Rect, colors[kBC_Light]);
   pRenderDevice->RenderBeveledRect(thumbRect, bevel, colors[kBC_Light], GUIStandardColors::DarkGray, colors[kBC_Face]);

   if (armedPart == kGUIScrollBarPartButton1 && armedPart == mouseOverPart)
   {
      pRenderDevice->RenderBeveledRect(btn1Rect, bevel, GUIStandardColors::DarkGray, colors[kBC_Light], colors[kBC_Face]);
   }
   else
   {
      pRenderDevice->RenderBeveledRect(btn1Rect, bevel, colors[kBC_Light], GUIStandardColors::DarkGray, colors[kBC_Face]);
   }

   if (armedPart == kGUIScrollBarPartButton2 && armedPart == mouseOverPart)
   {
      pRenderDevice->RenderBeveledRect(btn2Rect, bevel, colors[kBC_Shadow], colors[kBC_Light], colors[kBC_Face]);
   }
   else
   {
      pRenderDevice->RenderBeveledRect(btn2Rect, bevel, colors[kBC_Light], colors[kBC_Shadow], colors[kBC_Face]);
   }

   return S_OK;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIDialogElement * pDialogElement)
{
   tGUISize size(GetPreferredSize(static_cast<IGUIContainerElement*>(pDialogElement)));

   uint captionHeight;
   if (pDialogElement->GetCaptionHeight(&captionHeight) == S_OK)
   {
      size.height += captionHeight;
   }
   else
   {
      tGUIString title;
      if (pDialogElement->GetTitle(&title) == S_OK)
      {
         cAutoIPtr<IGUIFont> pFont;
         if (GetFont(pDialogElement, &pFont) == S_OK)
         {
            tRect rect(0,0,0,0);
            pFont->RenderText(title.c_str(), title.length(), &rect, kRT_CalcRect, GUIStandardColors::White);

            captionHeight = rect.GetHeight();

            pDialogElement->SetCaptionHeight(captionHeight);
            size.height += captionHeight;
         }
      }
   }

   return size;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUILabelElement * pLabelElement)
{
   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pLabelElement, &pFont) == S_OK)
   {
      tGUIString text;
      if (pLabelElement->GetText(&text) == S_OK)
      {
         tRect rect(0,0,0,0);
         pFont->RenderText(text.c_str(), text.length(), &rect, kRT_CalcRect, GUIStandardColors::White);

         return tGUISize(static_cast<tGUISizeType>(rect.GetWidth()), static_cast<tGUISizeType>(rect.GetHeight()));
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIPanelElement * pPanelElement)
{
   return GetPreferredSize(static_cast<IGUIContainerElement*>(pPanelElement));
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUITextEditElement * pTextEditElement)
{
   uint editSize;
   if (pTextEditElement->GetEditSize(&editSize) != S_OK)
   {
      editSize = kDefaultEditSize;
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pTextEditElement, &pFont) == S_OK)
   {
      char * psz = reinterpret_cast<char *>(alloca(editSize * sizeof(char)));
      memset(psz, 'M', editSize * sizeof(char));

      tRect rect(0,0,0,0);
      pFont->RenderText(psz, editSize, &rect, kRT_CalcRect | kRT_SingleLine, GUIStandardColors::White);

      return tGUISize(static_cast<tGUISizeType>(rect.GetWidth() + (kHorzInset * 2)),
                      static_cast<tGUISizeType>(rect.GetHeight() + (kVertInset * 2)));
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIScrollBarElement * pScrollBarElement)
{
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


///////////////////////////////////////////////////////////////////////////////

tResult GUIBeveledRendererCreate(void * pReserved, IGUIElementRenderer * * ppRenderer)
{
   static cAutoIPtr<IGUIElementRenderer> pSingleInstance;

   // Since the beveled renderer is stateless, a single instance can serve all GUI elements
   if (!pSingleInstance)
   {
      pSingleInstance = static_cast<IGUIElementRenderer*>(new cGUIBeveledRenderer);
      if (!pSingleInstance)
      {
         return E_OUTOFMEMORY;
      }
   }

   return pSingleInstance.GetPointer(ppRenderer);
}

AUTOREGISTER_GUIRENDERERFACTORYFN(beveled, GUIBeveledRendererCreate);

///////////////////////////////////////////////////////////////////////////////
