///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibutton.h"

#include "font.h"
#include "color.h"
#include "render.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElement
//

///////////////////////////////////////

cGUIButtonElement::cGUIButtonElement()
 : m_id(""),
   m_bHasFocus(false),
   m_bVisible(true),
   m_bEnabled(true),
   m_position(0,0),
   m_size(0,0),
   m_bArmed(false),
   m_bMouseOver(false),
   m_text("")
{
}

///////////////////////////////////////

cGUIButtonElement::~cGUIButtonElement()
{
}

///////////////////////////////////////

const char * cGUIButtonElement::GetId() const
{
   return m_id;
}

///////////////////////////////////////

void cGUIButtonElement::SetId(const char * pszId)
{
   m_id = pszId;
}

///////////////////////////////////////

bool cGUIButtonElement::HasFocus() const
{
   return m_bHasFocus;
}

///////////////////////////////////////

void cGUIButtonElement::SetFocus(bool bFocus)
{
   m_bHasFocus = bFocus;
}

///////////////////////////////////////

bool cGUIButtonElement::IsVisible() const
{
   return m_bVisible;
}

///////////////////////////////////////

void cGUIButtonElement::SetVisible(bool bVisible)
{
   m_bVisible = bVisible;
}

///////////////////////////////////////

bool cGUIButtonElement::IsEnabled() const
{
   return m_bEnabled;
}

///////////////////////////////////////

void cGUIButtonElement::SetEnabled(bool bEnabled)
{
   m_bEnabled = bEnabled;
}

///////////////////////////////////////

tResult cGUIButtonElement::GetParent(IGUIElement * * ppParent)
{
   return m_pParent.GetPointer(ppParent);
}

///////////////////////////////////////

tResult cGUIButtonElement::SetParent(IGUIElement * pParent)
{
   SafeRelease(m_pParent);
   m_pParent = CTAddRef(pParent);
   return S_OK;
}

///////////////////////////////////////

tGUIPoint cGUIButtonElement::GetPosition() const
{
   return m_position;
}

///////////////////////////////////////

void cGUIButtonElement::SetPosition(const tGUIPoint & point)
{
   m_position = point;
}

///////////////////////////////////////

tGUISize cGUIButtonElement::GetSize() const
{
   return m_size;
}

///////////////////////////////////////

void cGUIButtonElement::SetSize(const tGUISize & size)
{
   m_size = size;
}

///////////////////////////////////////

bool cGUIButtonElement::Contains(const tGUIPoint & point)
{
   return false; // TODO
}

///////////////////////////////////////

tResult cGUIButtonElement::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}

///////////////////////////////////////

tResult cGUIButtonElement::GetRendererClass(tGUIString * pRendererClass)
{
   if (pRendererClass == NULL)
      return E_POINTER;
   *pRendererClass = "button";
   return S_OK;
}

///////////////////////////////////////

tResult cGUIButtonElement::GetRenderer(IGUIElementRenderer * * ppRenderer)
{
   return m_pRenderer.GetPointer(ppRenderer);
}

///////////////////////////////////////

tResult cGUIButtonElement::SetRenderer(IGUIElementRenderer * pRenderer)
{
   SafeRelease(m_pRenderer);
   m_pRenderer = CTAddRef(pRenderer);
   return S_OK;
}

///////////////////////////////////////

bool cGUIButtonElement::IsArmed() const
{
   return m_bArmed;
}

///////////////////////////////////////

void cGUIButtonElement::SetArmed(bool bArmed)
{
   m_bArmed = bArmed;
}

///////////////////////////////////////

bool cGUIButtonElement::IsMouseOver() const
{
   return m_bMouseOver;
}

///////////////////////////////////////

void cGUIButtonElement::SetMouseOver(bool bMouseOver)
{
   m_bMouseOver = bMouseOver;
}

///////////////////////////////////////

const char * cGUIButtonElement::GetText() const
{
   return m_text;
}

///////////////////////////////////////

void cGUIButtonElement::SetText(const char * pszText)
{
   m_text = pszText;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(button, cGUIButtonElementFactory);

tResult cGUIButtonElementFactory::CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), "button") == 0)
      {
         cAutoIPtr<IGUIButtonElement> pButton = static_cast<IGUIButtonElement *>(new cGUIButtonElement);
         if (!!pButton)
         {
            if (pXmlElement->Attribute("text"))
            {
               pButton->SetText(pXmlElement->Attribute("text"));
            }

            *ppElement = CTAddRef(pButton);
            return S_OK;
         }
      }
   }
   else
   {
      *ppElement = static_cast<IGUIButtonElement *>(new cGUIButtonElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonRenderer
//

///////////////////////////////////////

cGUIButtonRenderer::cGUIButtonRenderer()
 : m_pFont(FontCreateDefault())
{
}

///////////////////////////////////////

cGUIButtonRenderer::~cGUIButtonRenderer()
{
}

///////////////////////////////////////

tResult cGUIButtonRenderer::Render(IGUIElement * pElement, IRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   // TODO

   cAutoIPtr<IGUIButtonElement> pButton;
   if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButton) == S_OK)
   {
      tGUIPoint pos = pButton->GetPosition();
      tGUISize size = pButton->GetSize();

      tRect rect(pos.x, pos.y, pos.x + size.width, pos.y + size.height);
      m_pFont->DrawText(pButton->GetText(), -1, kDT_NoClip, &rect, cColor(1,1,1,1));

      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUIButtonRenderer::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      cAutoIPtr<IGUIButtonElement> pButton;
      if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButton) == S_OK)
      {
         tRect rect(0,0,0,0);
         m_pFont->DrawText(pButton->GetText(), -1, kDT_CalcRect, &rect, cColor(1,1,1,1));
         return tGUISize(rect.GetWidth(), rect.GetHeight());
      }
   }
   return tGUISize(0,0);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(button, cGUIButtonRendererFactory);

tResult cGUIButtonRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   *ppRenderer = static_cast<IGUIElementRenderer *>(new cGUIButtonRenderer);
   return (*ppRenderer != NULL) ? S_OK : E_OUTOFMEMORY;
}

///////////////////////////////////////////////////////////////////////////////
