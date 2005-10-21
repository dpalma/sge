///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibeveledrenderer.h"
#include "guielementtools.h"
#include "guistrings.h"
#include "scriptapi.h"

#include "color.h"

#include "globalobj.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBeveledRenderer
//

static const int g_bevel = 2;

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

tResult cGUIBeveledRenderer::Render(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice)
{
   if (pRenderDevice == NULL || pElement == NULL)
   {
      return E_POINTER;
   }

   {
      cAutoIPtr<IGUIButtonElement> pButtonElement;
      if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButtonElement) == S_OK)
      {
         return Render(pButtonElement, pRenderDevice);
      }
   }

   {
      cAutoIPtr<IGUIDialogElement> pDialogElement;
      if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialogElement) == S_OK)
      {
         return Render(pDialogElement, pRenderDevice);
      }
   }

   {
      cAutoIPtr<IGUILabelElement> pLabelElement;
      if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabelElement) == S_OK)
      {
         return Render(pLabelElement, pRenderDevice);
      }
   }

   {
      cAutoIPtr<IGUIPanelElement> pPanelElement;
      if (pElement->QueryInterface(IID_IGUIPanelElement, (void**)&pPanelElement) == S_OK)
      {
         return Render(pPanelElement, pRenderDevice);
      }
   }

   {
      cAutoIPtr<IGUITextEditElement> pTextEditElement;
      if (pElement->QueryInterface(IID_IGUITextEditElement, (void**)&pTextEditElement) == S_OK)
      {
         return Render(pTextEditElement, pRenderDevice);
      }
   }

   {
      cAutoIPtr<IGUIScrollBarElement> pScrollBarElement;
      if (pElement->QueryInterface(IID_IGUIScrollBarElement, (void**)&pScrollBarElement) == S_OK)
      {
         return Render(pScrollBarElement, pRenderDevice);
      }
   }

   return E_FAIL;
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
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::GetFont(IGUIElement * pElement,
                                     IGUIFont * * ppFont)
{
   if (pElement == NULL || ppFont == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      cGUIFontDesc fontDesc;
      if (pStyle->GetFontDesc(&fontDesc) == S_OK)
      {
         UseGlobal(GUIFontFactory);
         return pGUIFontFactory->CreateFont(fontDesc, ppFont);
      }
   }

   UseGlobal(GUIContext);
   return pGUIContext->GetDefaultFont(ppFont);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIButtonElement * pButtonElement, IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pButtonElement);
   tGUISize size = pButtonElement->GetSize();

   bool bPressed = false;

   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   if (pButtonElement->IsArmed() && pButtonElement->IsMouseOver())
   {
      pRenderDevice->RenderBeveledRect(rect, g_bevel, tGUIColor::DarkGray, tGUIColor::LightGray, tGUIColor::Gray);
      bPressed = true;
   }
   else
   {
      pRenderDevice->RenderBeveledRect(rect, g_bevel, tGUIColor::LightGray, tGUIColor::DarkGray, tGUIColor::Gray);
   }

   tGUIString text;
   cAutoIPtr<IGUIFont> pFont;
   if (pButtonElement->GetText(&text) == S_OK
      && GetFont(pButtonElement, &pFont) == S_OK)
   {
      uint renderTextFlags = kRT_Center | kRT_VCenter | kRT_SingleLine;

      cAutoIPtr<IGUIStyle> pStyle;
      if (pButtonElement->GetStyle(&pStyle) == S_OK)
      {
         uint dropShadow = 0;
         if (pStyle->GetAttribute(kAttribDropShadow, &dropShadow) == S_OK
            && dropShadow != 0)
         {
            renderTextFlags |= kRT_DropShadow;
         }
      }

      if (bPressed)
      {
         rect.left += g_bevel;
         rect.top += g_bevel;
      }

      pFont->RenderText(text.c_str(), text.length(), &rect, renderTextFlags, tGUIColor::White);

      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIDialogElement * pDialogElement, IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pDialogElement);
   tGUISize size = pDialogElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   tGUIColor topLeft(tGUIColor::LightGray);
   tGUIColor bottomRight(tGUIColor::DarkGray);
   tGUIColor face(tGUIColor::Gray);
   tGUIColor caption(tGUIColor::Blue);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pDialogElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetAttribute("frame-top-left-color", &topLeft);
      pStyle->GetAttribute("frame-bottom-right-color", &bottomRight);
      pStyle->GetAttribute("frame-face-color", &face);
      pStyle->GetAttribute("caption-color", &caption);
   }

   pRenderDevice->RenderBeveledRect(rect, g_bevel, topLeft, bottomRight, face);

   uint captionHeight;
   if ((pDialogElement->GetCaptionHeight(&captionHeight) == S_OK)
      && (captionHeight > 0))
   {
      tGUIRect captionRect(rect);

      captionRect.left += g_bevel;
      captionRect.top += g_bevel;
      captionRect.right -= g_bevel;
      captionRect.bottom = captionRect.top + captionHeight;

      pRenderDevice->RenderSolidRect(captionRect, caption);

      cAutoIPtr<IGUIFont> pFont;
      if (GetFont(pDialogElement, &pFont) == S_OK)
      {
         tGUIString title;
         if (pDialogElement->GetTitle(&title) == S_OK)
         {
            pFont->RenderText(title.c_str(), -1, &captionRect, 0, tGUIColor::White);
         }
      }
   }

   return GUIElementRenderChildren(pDialogElement, pRenderDevice);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUILabelElement * pLabelElement, IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pLabelElement);
   tGUISize size = pLabelElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   tGUIColor color(tGUIColor::Black);

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
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIPanelElement * pPanelElement, IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pPanelElement);
   tGUISize size = pPanelElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   pRenderDevice->RenderBeveledRect(rect, g_bevel, tGUIColor::LightGray, tGUIColor::DarkGray, tGUIColor::Gray);

   return GUIElementRenderChildren(pPanelElement, pRenderDevice);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUITextEditElement * pTextEditElement, IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pTextEditElement);
   tGUISize size = pTextEditElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   pRenderDevice->RenderBeveledRect(rect, g_bevel, tGUIColor::DarkGray, tGUIColor::Gray, tGUIColor::White);

   rect.left += g_bevel + kHorzInset;
   rect.top += kVertInset;
   rect.right -= g_bevel + kHorzInset;
   rect.bottom -= kVertInset;

   pRenderDevice->PushScissorRect(rect);

   tGUIColor textColor(tGUIColor::Black);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pTextEditElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetForegroundColor(&textColor);
   }

   uint selStart, selEnd;
   Verify(pTextEditElement->GetSelection(&selStart, &selEnd) == S_OK);

   tGUIString text;
   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pTextEditElement, &pFont) == S_OK
      && pTextEditElement->GetText(&text) == S_OK)
   {
      // Determine the width of the text up to the cursor
      tRect leftOfCursor(0,0,0,0);
      pFont->RenderText(text.c_str(), selEnd, &leftOfCursor,
         kRT_NoClip | kRT_CalcRect, tGUIColor::White);

      // Offset the left edge so that the cursor is always in view.
      if (leftOfCursor.GetWidth() >= rect.GetWidth())
      {
         rect.left -= leftOfCursor.GetWidth() - rect.GetWidth() + kCursorWidth;
      }

      pFont->RenderText(text.c_str(), text.length(), &rect, kRT_NoClip, textColor);

      // Render the cursor if this widget has focus and its blink cycle is on
      if (pTextEditElement->HasFocus() && pTextEditElement->ShowBlinkingCursor())
      {
         tGUIRect cursorRect(
            rect.left + leftOfCursor.GetWidth(),
            rect.top + 1,
            rect.left + leftOfCursor.GetWidth() + kCursorWidth,
            rect.bottom - 1);

         pRenderDevice->RenderSolidRect(cursorRect, tGUIColor::Black);
      }
   }

   pRenderDevice->PopScissorRect();

   pTextEditElement->UpdateBlinkingCursor();

   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIScrollBarElement * pScrollBarElement,
                                    IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pScrollBarElement);
   tGUISize size = pScrollBarElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   uint rangeMin, rangeMax, position;
   Verify(pScrollBarElement->GetRange(&rangeMin, &rangeMax) == S_OK);
   Verify(pScrollBarElement->GetPosition(&position) == S_OK);
   Assert(position >= rangeMin && position <= rangeMax);

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

   int offsetx = Round(pos.x), offsety = Round(pos.y);
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

   pRenderDevice->RenderSolidRect(track1Rect, tGUIColor::LightGray);
   pRenderDevice->RenderSolidRect(track2Rect, tGUIColor::LightGray);
   pRenderDevice->RenderBeveledRect(thumbRect, g_bevel, tGUIColor::LightGray, tGUIColor::DarkGray, tGUIColor::Gray);

   if (armedPart == kGUIScrollBarPartButton1 && armedPart == mouseOverPart)
   {
      pRenderDevice->RenderBeveledRect(btn1Rect, g_bevel, tGUIColor::DarkGray, tGUIColor::LightGray, tGUIColor::Gray);
   }
   else
   {
      pRenderDevice->RenderBeveledRect(btn1Rect, g_bevel, tGUIColor::LightGray, tGUIColor::DarkGray, tGUIColor::Gray);
   }

   if (armedPart == kGUIScrollBarPartButton2 && armedPart == mouseOverPart)
   {
      pRenderDevice->RenderBeveledRect(btn2Rect, g_bevel, tGUIColor::DarkGray, tGUIColor::LightGray, tGUIColor::Gray);
   }
   else
   {
      pRenderDevice->RenderBeveledRect(btn2Rect, g_bevel, tGUIColor::LightGray, tGUIColor::DarkGray, tGUIColor::Gray);
   }

   return S_OK;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIButtonElement * pButtonElement)
{
   tGUIString text;
   cAutoIPtr<IGUIFont> pFont;
   if (pButtonElement->GetText(&text) == S_OK
      && GetFont(pButtonElement, &pFont) == S_OK)
   {
      tRect rect(0,0,0,0);
      pFont->RenderText(text.c_str(), text.length(), &rect, kRT_CalcRect, tGUIColor::White);

      return tGUISize(static_cast<tGUISizeType>(rect.GetWidth() + rect.GetHeight()),
                      rect.GetHeight() * 1.5f);
   }

   return tGUISize(0,0);
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
            pFont->RenderText(title.c_str(), title.length(), &rect, kRT_CalcRect, tGUIColor::White);

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
         pFont->RenderText(text.c_str(), text.length(), &rect, kRT_CalcRect, tGUIColor::White);

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
      pFont->RenderText(psz, editSize, &rect, kRT_CalcRect | kRT_SingleLine, tGUIColor::White);

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

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIContainerElement * pContainerElement)
{
   cAutoIPtr<IGUILayoutManager> pLayout;
   if (pContainerElement->GetLayout(&pLayout) == S_OK)
   {
      tGUISize size;
      if (pLayout->GetPreferredSize(pContainerElement, &size) == S_OK)
      {
         return size;
      }
   }
   return tGUISize(0,0);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBeveledRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(beveled, cGUIBeveledRendererFactory);

////////////////////////////////////////

tResult cGUIBeveledRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   // Since the renderer is stateless, a single instance can serve all GUI elements
   if (!m_pStatelessBeveledRenderer)
   {
      m_pStatelessBeveledRenderer = static_cast<IGUIElementRenderer *>(new cGUIBeveledRenderer);
      if (!m_pStatelessBeveledRenderer)
      {
         return E_OUTOFMEMORY;
      }
   }

   *ppRenderer = CTAddRef(m_pStatelessBeveledRenderer);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
