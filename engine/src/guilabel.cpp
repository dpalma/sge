///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilabel.h"
#include "guielementbasetem.h"

#include "font.h"
#include "color.h"
#include "render.h"

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
      return E_POINTER;
   *pRendererClass = "label";
   return S_OK;
}

///////////////////////////////////////

const char * cGUILabelElement::GetText() const
{
   return m_text;
}

///////////////////////////////////////

void cGUILabelElement::SetText(const char * pszText)
{
   m_text = pszText;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(label, cGUILabelElementFactory);

tResult cGUILabelElementFactory::CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement)
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
// CLASS: cGUILabelRenderer
//

///////////////////////////////////////

cGUILabelRenderer::cGUILabelRenderer()
 : m_pFont(FontCreateDefault())
{
}

///////////////////////////////////////

cGUILabelRenderer::~cGUILabelRenderer()
{
}

///////////////////////////////////////

tResult cGUILabelRenderer::Render(IGUIElement * pElement, IRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUILabelElement> pLabel;
   if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabel) == S_OK)
   {
      tGUIPoint pos = pLabel->GetPosition();
      tGUISize size = pLabel->GetSize();

      tRect rect(pos.x, pos.y, pos.x + size.width, pos.y + size.height);
      m_pFont->DrawText(pLabel->GetText(), -1, kDT_NoClip, &rect, cColor(1,1,1,1));
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUILabelRenderer::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      cAutoIPtr<IGUILabelElement> pLabel;
      if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabel) == S_OK)
      {
         tRect rect(0,0,0,0);
         m_pFont->DrawText(pLabel->GetText(), -1, kDT_CalcRect, &rect, cColor(1,1,1,1));
         return tGUISize(rect.GetWidth(), rect.GetHeight());
      }
   }
   return tGUISize(0,0);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(label, cGUILabelRendererFactory);

tResult cGUILabelRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   *ppRenderer = static_cast<IGUIElementRenderer *>(new cGUILabelRenderer);
   return (*ppRenderer != NULL) ? S_OK : E_OUTOFMEMORY;
}

///////////////////////////////////////////////////////////////////////////////
