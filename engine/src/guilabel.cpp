///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilabel.h"

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
 : m_id(""),
   m_bVisible(true),
   m_bEnabled(true),
   m_position(0,0),
   m_size(0,0),
   m_text("")
{
}

///////////////////////////////////////

cGUILabelElement::~cGUILabelElement()
{
}

///////////////////////////////////////

const char * cGUILabelElement::GetId() const
{
   return m_id;
}

///////////////////////////////////////

void cGUILabelElement::SetId(const char * pszId)
{
   m_id = pszId;
}

///////////////////////////////////////

bool cGUILabelElement::HasFocus() const
{
   return false;
}

///////////////////////////////////////

void cGUILabelElement::SetFocus(bool /*bFocus*/)
{
}

///////////////////////////////////////

bool cGUILabelElement::IsVisible() const
{
   return m_bVisible;
}

///////////////////////////////////////

void cGUILabelElement::SetVisible(bool bVisible)
{
   m_bVisible = bVisible;
}

///////////////////////////////////////

bool cGUILabelElement::IsEnabled() const
{
   return m_bEnabled;
}

///////////////////////////////////////

void cGUILabelElement::SetEnabled(bool bEnabled)
{
   m_bEnabled = bEnabled;
}

///////////////////////////////////////

tResult cGUILabelElement::GetParent(IGUIElement * * ppParent)
{
   return m_pParent.GetPointer(ppParent);
}

///////////////////////////////////////

tResult cGUILabelElement::SetParent(IGUIElement * pParent)
{
   SafeRelease(m_pParent);
   m_pParent = CTAddRef(pParent);
   return S_OK;
}

///////////////////////////////////////

tGUIPoint cGUILabelElement::GetPosition() const
{
   return m_position;
}

///////////////////////////////////////

void cGUILabelElement::SetPosition(const tGUIPoint & point)
{
   m_position = point;
}

///////////////////////////////////////

tGUISize cGUILabelElement::GetSize() const
{
   return m_size;
}

///////////////////////////////////////

void cGUILabelElement::SetSize(const tGUISize & size)
{
   m_size = size;
}

///////////////////////////////////////

bool cGUILabelElement::Contains(const tGUIPoint & point)
{
   return false; // TODO
}

///////////////////////////////////////

tResult cGUILabelElement::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
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

tResult cGUILabelElement::GetRenderer(IGUIElementRenderer * * ppRenderer)
{
   return m_pRenderer.GetPointer(ppRenderer);
}

///////////////////////////////////////

tResult cGUILabelElement::SetRenderer(IGUIElementRenderer * pRenderer)
{
   SafeRelease(m_pRenderer);
   m_pRenderer = CTAddRef(pRenderer);
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
