///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilabel.h"
#include "guielementbasetem.h"
#include "guielementtools.h"

#include "font.h"
#include "color.h"
#include "render.h"

#include "globalobj.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElement
//

///////////////////////////////////////

cGUILabelElement::cGUILabelElement()
 : m_text("")
{
}

///////////////////////////////////////

cGUILabelElement::~cGUILabelElement()
{
}

///////////////////////////////////////

tResult cGUILabelElement::GetRendererClass(tGUIString * pRendererClass)
{
   if (pRendererClass == NULL)
   {
      return E_POINTER;
   }
   *pRendererClass = "label";
   return S_OK;
}

///////////////////////////////////////

tResult cGUILabelElement::GetText(tGUIString * pText)
{
   if (pText == NULL)
   {
      return E_POINTER;
   }
   *pText = m_text;
   return S_OK;
}

///////////////////////////////////////

tResult cGUILabelElement::SetText(const char * pszText)
{
   if (pszText == NULL)
   {
      return E_POINTER;
   }
   m_text = pszText;
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(label, cGUILabelElementFactory);

tResult cGUILabelElementFactory::CreateElement(const TiXmlElement * pXmlElement, 
                                               IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), "label") == 0)
      {
         cAutoIPtr<IGUILabelElement> pLabel = static_cast<IGUILabelElement *>(new cGUILabelElement);
         if (!!pLabel)
         {
            GUIElementStandardAttributes(pXmlElement, pLabel);

            if (pXmlElement->Attribute("text"))
            {
               pLabel->SetText(pXmlElement->Attribute("text"));
            }

            *ppElement = CTAddRef(pLabel);
            return S_OK;
         }
      }
   }
   else
   {
      *ppElement = static_cast<IGUILabelElement *>(new cGUILabelElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelStatelessRenderer
//

///////////////////////////////////////

cGUILabelStatelessRenderer::cGUILabelStatelessRenderer()
{
}

///////////////////////////////////////

cGUILabelStatelessRenderer::~cGUILabelStatelessRenderer()
{
}

///////////////////////////////////////

tResult cGUILabelStatelessRenderer::Render(IGUIElement * pElement, IRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUILabelElement> pLabel;
   if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabel) == S_OK)
   {
      tGUIPoint pos = GUIElementAbsolutePosition(pLabel);
      tGUISize size = pLabel->GetSize();

      tRect rect(pos.x, pos.y, pos.x + size.width, pos.y + size.height);

      tGUIColor color(tGUIColor::White);

      cAutoIPtr<IRenderFont> pFont;

      cAutoIPtr<IGUIStyle> pStyle;
      if (pElement->GetStyle(&pStyle) == S_OK)
      {
         pStyle->GetForegroundColor(&color);
         pStyle->GetFont(&pFont);
      }

      if (!pFont)
      {
         UseGlobal(GUIRenderingTools);
         pGUIRenderingTools->GetDefaultFont(&pFont);
      }

      tGUIString text;
      if (pLabel->GetText(&text) == S_OK)
      {
         pFont->DrawText(text, text.length(), kDT_NoClip, &rect, color);
      }

      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUILabelStatelessRenderer::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      cAutoIPtr<IGUILabelElement> pLabel;
      if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabel) == S_OK)
      {
         cAutoIPtr<IRenderFont> pFont;

         cAutoIPtr<IGUIStyle> pStyle;
         if (pElement->GetStyle(&pStyle) == S_OK)
         {
            pStyle->GetFont(&pFont);
         }

         if (!pFont)
         {
            UseGlobal(GUIRenderingTools);
            pGUIRenderingTools->GetDefaultFont(&pFont);
         }

         tGUIString text;
         if (pLabel->GetText(&text) == S_OK)
         {
            tRect rect(0,0,0,0);
            pFont->DrawText(text, text.length(), kDT_CalcRect, &rect, tGUIColor::White);

            return tGUISize(rect.GetWidth(), rect.GetHeight());
         }
      }
   }
   return tGUISize(0,0);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelStatelessRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(label, cGUILabelStatelessRendererFactory);

tResult cGUILabelStatelessRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, 
                                                          IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   if (!m_pStatelessLabelRenderer)
   {
      m_pStatelessLabelRenderer = static_cast<IGUIElementRenderer *>(new cGUILabelStatelessRenderer);
      if (!m_pStatelessLabelRenderer)
      {
         return E_OUTOFMEMORY;
      }
   }

   *ppRenderer = CTAddRef(m_pStatelessLabelRenderer);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
