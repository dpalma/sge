///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibeveledrenderer.h"
#include "guielementtools.h"
#include "scriptapi.h"
#include "guistrings.h"

#include "font.h"
#include "color.h"

#include "globalobj.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBeveledRenderer
//

static const int g_bevel = 2;
static const float g_bevelf = static_cast<float>(g_bevel);

///////////////////////////////////////

cGUIBeveledRenderer::cGUIBeveledRenderer()
{
   FontCreateDefault(&m_pDefaultFont);
}

///////////////////////////////////////

cGUIBeveledRenderer::~cGUIBeveledRenderer()
{
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   {
      cAutoIPtr<IGUIButtonElement> pButtonElement;
      if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButtonElement) == S_OK)
      {
         return Render(pButtonElement);
      }
   }

   {
      cAutoIPtr<IGUIDialogElement> pDialogElement;
      if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialogElement) == S_OK)
      {
         return Render(pDialogElement);
      }
   }

   {
      cAutoIPtr<IGUILabelElement> pLabelElement;
      if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabelElement) == S_OK)
      {
         return Render(pLabelElement);
      }
   }

   {
      cAutoIPtr<IGUIPanelElement> pPanelElement;
      if (pElement->QueryInterface(IID_IGUIPanelElement, (void**)&pPanelElement) == S_OK)
      {
         return Render(pPanelElement);
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
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIButtonElement * pButtonElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pButtonElement);
   tGUISize size = pButtonElement->GetSize();

   tVec2 textOffset(0,0);

   tRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));
   tGUIRect rect2(rect.left, rect.top, rect.right, rect.bottom);

   if (pButtonElement->IsArmed() && pButtonElement->IsMouseOver())
   {
      GlRenderBevelledRect(rect2, g_bevel, tGUIColor::DarkGray, tGUIColor::LightGray, tGUIColor::Gray);
      textOffset = tVec2(g_bevelf, g_bevelf);
   }
   else
   {
      GlRenderBevelledRect(rect2, g_bevel, tGUIColor::LightGray, tGUIColor::DarkGray, tGUIColor::Gray);
   }

   cAutoIPtr<IRenderFont> pFont;
   if (GetFont(pButtonElement, &pFont) == S_OK)
   {
      uint drawTextFlags = kDT_Center | kDT_VCenter | kDT_SingleLine;

      cAutoIPtr<IGUIStyle> pStyle;
      if (pButtonElement->GetStyle(&pStyle) == S_OK)
      {
         uint dropShadow = 0;
         if (pStyle->GetAttribute(kAttribDropShadow, &dropShadow) == S_OK
            && dropShadow != 0)
         {
            drawTextFlags |= kDT_DropShadow;
         }
      }

      rect.left += Round(textOffset.x);
      rect.top += Round(textOffset.y);
      pFont->DrawText(pButtonElement->GetText(), -1, drawTextFlags, &rect, tGUIColor::White);
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIDialogElement * pDialogElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pDialogElement);
   tGUISize size = pDialogElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   UseGlobal(GUIRenderingTools);

   tGUIColor topLeft(tGUIColor::LightGray);
   tGUIColor bottomRight(tGUIColor::DarkGray);
   tGUIColor face(tGUIColor::Gray);
   tGUIColor caption(tGUIColor::Blue);

   cAutoIPtr<IRenderFont> pFont;

   cAutoIPtr<IGUIStyle> pStyle;
   if (pDialogElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetFont(&pFont);
      pStyle->GetAttribute("frame-top-left-color", &topLeft);
      pStyle->GetAttribute("frame-bottom-right-color", &bottomRight);
      pStyle->GetAttribute("frame-face-color", &face);
      pStyle->GetAttribute("caption-color", &caption);
   }

   if (!pFont)
   {
      pFont = CTAddRef(m_pDefaultFont);
   }

   GlRenderBevelledRect(rect, g_bevel, topLeft, bottomRight, face);

   uint captionHeight;
   if ((pDialogElement->GetCaptionHeight(&captionHeight) == S_OK)
      && (captionHeight > 0))
   {
      tGUIRect captionRect(rect);

      captionRect.left += g_bevel;
      captionRect.top += g_bevel;
      captionRect.right -= g_bevel;
      captionRect.bottom = captionRect.top + captionHeight;

      GlRenderBevelledRect(captionRect, 0, caption, caption, caption);

      tGUIString title;
      if (pDialogElement->GetTitle(&title) == S_OK)
      {
         pFont->DrawText(title.c_str(), -1, 0, &captionRect, tGUIColor::White);
      }
   }

   return GUIElementRenderChildren(pDialogElement);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUILabelElement * pLabelElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pLabelElement);
   tGUISize size = pLabelElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   tGUIColor color(tGUIColor::Black);

   cAutoIPtr<IRenderFont> pFont;

   cAutoIPtr<IGUIStyle> pStyle;
   if (pLabelElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetForegroundColor(&color);
      pStyle->GetFont(&pFont);
   }

   if (!pFont)
   {
      return E_FAIL;
   }

   tGUIString text;
   if (pLabelElement->GetText(&text) == S_OK)
   {
      pFont->DrawText(text.c_str(), text.length(), kDT_NoClip, &rect, color);
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIPanelElement * pPanelElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pPanelElement);
   tGUISize size = pPanelElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   GlRenderBevelledRect(rect, g_bevel, tGUIColor::LightGray, tGUIColor::DarkGray, tGUIColor::Gray);

   return GUIElementRenderChildren(pPanelElement);
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIButtonElement * pButtonElement)
{
   cAutoIPtr<IRenderFont> pFont;
   if (GetFont(pButtonElement, &pFont) == S_OK)
   {
      tRect rect(0,0,0,0);
      pFont->DrawText(pButtonElement->GetText(), -1, kDT_CalcRect, &rect, tGUIColor::White);

      return tGUISize(static_cast<tGUISizeType>(rect.GetWidth() + rect.GetHeight()),
                      rect.GetHeight() * 1.5f);
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIDialogElement * pDialogElement)
{
   cAutoIPtr<IRenderFont> pFont;

   cAutoIPtr<IGUIStyle> pStyle;
   if (pDialogElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetFont(&pFont);
   }

   if (!pFont)
   {
      pFont = CTAddRef(m_pDefaultFont);
   }

   cAutoIPtr<IGUILayoutManager> pLayout;
   if (pDialogElement->GetLayout(&pLayout) == S_OK)
   {
      tGUISize size(0,0);
      if (pLayout->GetPreferredSize(pDialogElement, &size) == S_OK)
      {
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
               tRect rect(0,0,0,0);
               pFont->DrawText(title.c_str(), -1, kDT_CalcRect, &rect, tGUIColor::White);

               captionHeight = rect.GetHeight();

               pDialogElement->SetCaptionHeight(captionHeight);
               size.height += captionHeight;
            }
         }

         return size;
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUILabelElement * pLabelElement)
{
   cAutoIPtr<IRenderFont> pFont;
   if (GetFont(pLabelElement, &pFont) == S_OK)
   {
      tGUIString text;
      if (pLabelElement->GetText(&text) == S_OK)
      {
         tRect rect(0,0,0,0);
         pFont->DrawText(text.c_str(), text.length(), kDT_CalcRect, &rect, tGUIColor::White);

         return tGUISize((tGUISizeType)rect.GetWidth(), (tGUISizeType)rect.GetHeight());
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIPanelElement * pPanelElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::GetFont(IGUIElement * pElement,
                                     IRenderFont * * ppFont)
{
   if (pElement == NULL || ppFont == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      if (pStyle->GetFont(ppFont) == S_OK)
      {
         return S_OK;
      }
   }

   *ppFont = CTAddRef(m_pDefaultFont);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBeveledRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(beveled, cGUIBeveledRendererFactory);

tResult cGUIBeveledRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   *ppRenderer = static_cast<IGUIElementRenderer *>(new cGUIBeveledRenderer);
   return (*ppRenderer != NULL) ? S_OK : E_OUTOFMEMORY;
}

///////////////////////////////////////////////////////////////////////////////
