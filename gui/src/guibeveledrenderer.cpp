///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibeveledrenderer.h"
#include "guielementtools.h"
#include "scriptapi.h"
#include "guistrings.h"

#include "color.h"

#include "globalobj.h"

#include <tinyxml.h>
#include <GL/glew.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBeveledRenderer
//

static const int g_bevel = 2;
static const float g_bevelf = static_cast<float>(g_bevel);

static const uint kDefaultEditSize = 20;
static const uint kVertInset = 2; // TODO make this part of the style
static const uint kHorzInset = 2; // TODO make this part of the style
static const uint kCursorWidth = 1;

///////////////////////////////////////

cGUIBeveledRenderer::cGUIBeveledRenderer()
{
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

   {
      cAutoIPtr<IGUITextEditElement> pTextEditElement;
      if (pElement->QueryInterface(IID_IGUITextEditElement, (void**)&pTextEditElement) == S_OK)
      {
         return Render(pTextEditElement);
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

   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pButtonElement, &pFont) == S_OK)
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

      rect.left += Round(textOffset.x);
      rect.top += Round(textOffset.y);
      pFont->RenderText(pButtonElement->GetText(), -1, &rect, renderTextFlags, tGUIColor::White);
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIBeveledRenderer::Render(IGUIDialogElement * pDialogElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pDialogElement);
   tGUISize size = pDialogElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   tGUIColor topLeft(tGUIColor::LightGray);
   tGUIColor bottomRight(tGUIColor::DarkGray);
   tGUIColor face(tGUIColor::Gray);
   tGUIColor caption(tGUIColor::Blue);

   cAutoIPtr<IGUIFont> pFont;

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
      pFont = CTAddRef(AccessDefaultFont());
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
         pFont->RenderText(title.c_str(), -1, &captionRect, 0, tGUIColor::White);
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

   cAutoIPtr<IGUIFont> pFont;

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
      pFont->RenderText(text.c_str(), text.length(), &rect, kRT_NoClip, color);
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

tResult cGUIBeveledRenderer::Render(IGUITextEditElement * pTextEditElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pTextEditElement);
   tGUISize size = pTextEditElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   GlRenderBevelledRect(rect, g_bevel, tGUIColor::DarkGray, tGUIColor::Gray, tGUIColor::White);

   rect.left += g_bevel + kHorzInset;
   rect.top += kVertInset;
   rect.right -= g_bevel + kHorzInset;
   rect.bottom -= kVertInset;

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   glEnable(GL_SCISSOR_TEST);
   glScissor(
      rect.left,
      // @HACK: the call to glOrtho made at the beginning of each UI render
      // cycle typically makes the UPPER left corner (0,0).  glScissor seems 
      // to assume that (0,0) is always the LOWER left corner.
      viewport[3] - rect.bottom,
      rect.GetWidth(),
      rect.GetHeight());

   tGUIColor textColor(tGUIColor::Black);

   cAutoIPtr<IGUIFont> pFont;

   cAutoIPtr<IGUIStyle> pStyle;
   if (pTextEditElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetForegroundColor(&textColor);
      pStyle->GetFont(&pFont);
   }

   if (!pFont)
   {
      pFont = CTAddRef(AccessDefaultFont());
   }

   uint selStart, selEnd;
   Verify(pTextEditElement->GetSelection(&selStart, &selEnd) == S_OK);

   // Determine the width of the text up to the cursor
   tRect leftOfCursor(0,0,0,0);
   pFont->RenderText(pTextEditElement->GetText(), selEnd, &leftOfCursor,
      kRT_NoClip | kRT_CalcRect, tGUIColor::White);

   // Offset the left edge so that the cursor is always in view.
   if (leftOfCursor.GetWidth() >= rect.GetWidth())
   {
      rect.left -= leftOfCursor.GetWidth() - rect.GetWidth() + kCursorWidth;
   }

   pFont->RenderText(pTextEditElement->GetText(), -1, &rect, kRT_NoClip, textColor);

   // Render the cursor if this widget has focus and its blink cycle is on
   if (pTextEditElement->HasFocus() && pTextEditElement->ShowBlinkingCursor())
   {
      tGUIRect cursorRect(
         rect.left + leftOfCursor.GetWidth(),
         rect.top + 1,
         rect.left + leftOfCursor.GetWidth() + kCursorWidth,
         rect.bottom - 1);

      GlRenderBevelledRect(cursorRect, 0, tGUIColor::Black, tGUIColor::Black, tGUIColor::Black);
   }

   glDisable(GL_SCISSOR_TEST);

   pTextEditElement->UpdateBlinkingCursor();

   return S_OK;
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIButtonElement * pButtonElement)
{
   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pButtonElement, &pFont) == S_OK)
   {
      tRect rect(0,0,0,0);
      pFont->RenderText(pButtonElement->GetText(), -1, &rect, kRT_CalcRect, tGUIColor::White);

      return tGUISize(static_cast<tGUISizeType>(rect.GetWidth() + rect.GetHeight()),
                      rect.GetHeight() * 1.5f);
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBeveledRenderer::GetPreferredSize(IGUIDialogElement * pDialogElement)
{
   cAutoIPtr<IGUIFont> pFont;

   cAutoIPtr<IGUIStyle> pStyle;
   if (pDialogElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetFont(&pFont);
   }

   if (!pFont)
   {
      pFont = CTAddRef(AccessDefaultFont());
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
               pFont->RenderText(title.c_str(), -1, &rect, kRT_CalcRect, tGUIColor::White);

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
   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pLabelElement, &pFont) == S_OK)
   {
      tGUIString text;
      if (pLabelElement->GetText(&text) == S_OK)
      {
         tRect rect(0,0,0,0);
         pFont->RenderText(text.c_str(), text.length(), &rect, kRT_CalcRect, tGUIColor::White);

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
      if (pStyle->GetFont(ppFont) == S_OK)
      {
         return S_OK;
      }
   }

   *ppFont = CTAddRef(AccessDefaultFont());
   return S_OK;
}

///////////////////////////////////////

IGUIFont * cGUIBeveledRenderer::AccessDefaultFont()
{
   if (!m_pDefaultFont)
   {
      GUIFontGetDefault(&m_pDefaultFont);
   }
   return m_pDefaultFont;
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
