///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibasicrenderer.h"
#include "guielementtools.h"
#include "guistrings.h"
#include "scriptapi.h"

#include "globalobj.h"

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

tResult cGUIBasicRenderer::Render(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
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

tResult cGUIBasicRenderer::GetPreferredSize(IGUIElement * pElement, tGUISize * pSize)
{
   if (pElement == NULL || pSize == NULL)
   {
      return E_POINTER;
   }

   *pSize = GetPreferredSize(pElement);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::ComputeClientArea(IGUIElement * pElement, tGUIRect * pClientArea)
{
   if (pElement == NULL || pClientArea == NULL)
   {
      return E_POINTER;
   }

   tGUISize size = pElement->GetSize();
   tGUIRect clientArea(g_bevel, g_bevel, Round(size.width - g_bevel), Round(size.height - g_bevel));

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
                  if (pFont->RenderText(title.c_str(), title.length(), &rect, kRT_CalcRect, tGUIColor::White) == S_OK)
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

tResult cGUIBasicRenderer::Render(IGUIButtonElement * pButtonElement, IGUIRenderDevice * pRenderDevice)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUIDialogElement * pDialogElement, IGUIRenderDevice * pRenderDevice)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUILabelElement * pLabelElement, IGUIRenderDevice * pRenderDevice)
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
      if (text.empty())
      {
         return S_FALSE;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUIPanelElement * pPanelElement, IGUIRenderDevice * pRenderDevice)
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
         pRenderDevice->RenderSolidRect(rect, bkColor);
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUITextEditElement * pTextEditElement, IGUIRenderDevice * pRenderDevice)
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

tResult GUIBasicRendererCreate(void * pReserved, IGUIElementRenderer * * ppRenderer)
{
   static cAutoIPtr<IGUIElementRenderer> pSingleInstance;

   // Since the basic renderer is stateless, a single instance can serve all GUI elements
   if (!pSingleInstance)
   {
      pSingleInstance = static_cast<IGUIElementRenderer*>(new cGUIBasicRenderer);
      if (!pSingleInstance)
      {
         return E_OUTOFMEMORY;
      }
   }

   return pSingleInstance.GetPointer(ppRenderer);
}

AUTOREGISTER_GUIRENDERERFACTORYFN(basic, GUIBasicRendererCreate);

///////////////////////////////////////////////////////////////////////////////
