///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibasicrenderer.h"
#include "guielementtools.h"
#include "guistyleapi.h"

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

const cGUIBasicRenderer::sMethodTableEntry cGUIBasicRenderer::gm_methodTable[] =
{
   { &IID_IGUILabelElement, &LabelRender, &LabelPreferredSize },
   { &IID_IGUIPanelElement, &PanelRender, &PanelPreferredSize },
};

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   for (int i = 0; i < _countof(gm_methodTable); i++)
   {
      cAutoIPtr<IGUIElement> pElement2;
      if (pElement->QueryInterface(*(gm_methodTable[i].pIID), (void**)&pElement2) == S_OK)
      {
         if (gm_methodTable[i].pfnRender != NULL)
         {
            return (this->*(gm_methodTable[i].pfnRender))(pElement2, pRenderDevice);
         }
         else
         {
            break;
         }
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::GetPreferredSize(IGUIElement * pElement, tGUISize * pSize)
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
            *pSize = (this->*(gm_methodTable[i].pfnPreferredSize))(pElement2);
            return S_OK;
         }
         else
         {
            break;
         }
      }
   }

   return E_FAIL;
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

tResult cGUIBasicRenderer::GetFont(IGUIElement * pElement,
                                   IGUIFont * * ppFont) const
{
   return GUIElementFont(pElement, ppFont);
}

///////////////////////////////////////

tResult cGUIBasicRenderer::LabelRender(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pElement);
   tGUISize size = pElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   tGUIColor color(GUIStandardColors::Black);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetForegroundColor(&color);
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GetFont(pElement, &pFont) == S_OK)
   {
      IGUILabelElement * pLabelElement = (IGUILabelElement *)pElement;

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

tGUISize cGUIBasicRenderer::LabelPreferredSize(IGUIElement * pElement) const
{
   cAutoIPtr<IGUIFont> pFont;
   if (GUIElementFont(pElement, &pFont) == S_OK)
   {
      tGUIString text;
      if (((IGUILabelElement*)pElement)->GetText(&text) == S_OK)
      {
         tRect rect(0,0,0,0);
         pFont->RenderText(text.c_str(), text.length(), &rect, kRT_CalcRect, GUIStandardColors::White);

         return tGUISize(static_cast<tGUISizeType>(rect.GetWidth()), static_cast<tGUISizeType>(rect.GetHeight()));
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBasicRenderer::PanelRender(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pElement);
   tGUISize size = pElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
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

tGUISize cGUIBasicRenderer::PanelPreferredSize(IGUIElement * pElement) const
{
   cAutoIPtr<IGUILayoutManager> pLayout;
   if (((IGUIPanelElement*)pElement)->GetLayout(&pLayout) == S_OK)
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
