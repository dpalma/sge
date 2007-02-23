///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibasicrenderer.h"
#include "guielementtools.h"
#include "gui/guistyleapi.h"

#include "render/renderapi.h"
#include "render/renderfontapi.h"

#include "tech/globalobj.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

extern void GUIRenderSolidRect(const tRecti & rect, const float color[4]);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBasicRenderer
//

///////////////////////////////////////

cGUIBasicRenderer::cGUIBasicRenderer()
{
}

///////////////////////////////////////

cGUIBasicRenderer::~cGUIBasicRenderer()
{
}

///////////////////////////////////////

#define METHOD_TABLE_ENTRY(ElementType) \
   { &IID_IGUI##ElementType##Element, &cGUIBasicRenderer::ElementType##Render, &cGUIBasicRenderer::ElementType##PreferredSize }

const cGUIBasicRenderer::sMethodTableEntry cGUIBasicRenderer::gm_methodTable[] =
{
   METHOD_TABLE_ENTRY(Label),
   METHOD_TABLE_ENTRY(Panel),
};

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUIElement * pElement, const tGUIPoint & position)
{
   if (pElement == NULL)
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
            return (this->*(gm_methodTable[i].pfnRender))(pElement2);
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

tResult cGUIBasicRenderer::GetPreferredSize(IGUIElement * pElement, const tGUISize & parentSize, tGUISize * pSize)
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

tResult cGUIBasicRenderer::AllocateBorderSpace(IGUIElement * pElement, tGUIRect * pRect)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::LabelRender(IGUIElement * pElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pElement);
   tGUISize size = pElement->GetSize();
   tGUIRect rect(FloatToInt(pos.x), FloatToInt(pos.y), FloatToInt(pos.x + size.width), FloatToInt(pos.y + size.height));

   tGUIColor color(GUIStandardColors::Black);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetForegroundColor(&color);
   }

   cAutoIPtr<IRenderFont> pFont;
   if (GUIElementFont(pElement, &pFont) == S_OK)
   {
      IGUILabelElement * pLabelElement = (IGUILabelElement *)pElement;

      tGUIString text;
      if (pLabelElement->GetText(&text) == S_OK)
      {
         pFont->RenderText(text.c_str(), text.length(), &rect, kRT_NoClip, color.GetPointer());
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
   cAutoIPtr<IRenderFont> pFont;
   if (GUIElementFont(pElement, &pFont) == S_OK)
   {
      tGUIString text;
      if (((IGUILabelElement*)pElement)->GetText(&text) == S_OK)
      {
         tGUIRect rect(0,0,0,0);
         pFont->RenderText(text.c_str(), text.length(), &rect, kRT_CalcRect, NULL);

         return tGUISize(static_cast<tGUISizeType>(rect.GetWidth()), static_cast<tGUISizeType>(rect.GetHeight()));
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

tResult cGUIBasicRenderer::PanelRender(IGUIElement * pElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pElement);
   tGUISize size = pElement->GetSize();
   tGUIRect rect(FloatToInt(pos.x), FloatToInt(pos.y), FloatToInt(pos.x + size.width), FloatToInt(pos.y + size.height));

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      tGUIColor bkColor;
      if (pStyle->GetBackgroundColor(&bkColor) == S_OK)
      {
         GUIRenderSolidRect(rect, bkColor.GetPointer());
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
