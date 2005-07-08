///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibasicrenderer.h"
#include "guielementtools.h"
#include "guistrings.h"
#include "scriptapi.h"

#include "color.h"

#include "globalobj.h"

#include <tinyxml.h>
#include <GL/glew.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBasicRenderer
//

static const int g_bevel = 2;

///////////////////////////////////////

cGUIBasicRenderer::cGUIBasicRenderer()
{
}

///////////////////////////////////////

cGUIBasicRenderer::~cGUIBasicRenderer()
{
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUIElement * pElement)
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

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUIElement * pElement)
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

tResult cGUIBasicRenderer::GetFont(IGUIElement * pElement,
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

tResult cGUIBasicRenderer::Render(IGUIButtonElement * pButtonElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUIDialogElement * pDialogElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUILabelElement * pLabelElement)
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

tResult cGUIBasicRenderer::Render(IGUIPanelElement * pPanelElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pPanelElement);
   tGUISize size = pPanelElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   cAutoIPtr<IGUIStyle> pStyle;
   if (pPanelElement->GetStyle(&pStyle) == S_OK)
   {
      tGUIColor bkColor;
      if (pStyle->GetBackgroundColor(&bkColor) == S_OK)
      {
         GlRenderBevelledRect(rect, 0, bkColor, bkColor, bkColor);
      }
   }

   return GUIElementRenderChildren(pPanelElement);
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUITextEditElement * pTextEditElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUIButtonElement * pButtonElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUIDialogElement * pDialogElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUILabelElement * pLabelElement)
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

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUIPanelElement * pPanelElement)
{
   return GetPreferredSize(static_cast<IGUIContainerElement*>(pPanelElement));
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUITextEditElement * pTextEditElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUIContainerElement * pContainerElement)
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
// CLASS: cGUIBasicRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(basic, cGUIBasicRendererFactory);

////////////////////////////////////////

tResult cGUIBasicRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   // Since the renderer is stateless, a single instance can serve all GUI elements
   if (!m_pStatelessBasicRenderer)
   {
      m_pStatelessBasicRenderer = static_cast<IGUIElementRenderer *>(new cGUIBasicRenderer);
      if (!m_pStatelessBasicRenderer)
      {
         return E_OUTOFMEMORY;
      }
   }

   *ppRenderer = CTAddRef(m_pStatelessBasicRenderer);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
