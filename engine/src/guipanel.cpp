///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipanel.h"
#include "guielementenum.h"

#include "font.h"
#include "color.h"
#include "render.h"

#include <tinyxml.h>
#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelElement
//

///////////////////////////////////////

cGUIPanelElement::cGUIPanelElement()
 : m_id(""),
   m_bVisible(true),
   m_bEnabled(true),
   m_position(0,0),
   m_size(0,0)
{
}

///////////////////////////////////////

cGUIPanelElement::~cGUIPanelElement()
{
   std::for_each(m_children.begin(), m_children.end(), CTInterfaceMethodRef(&IGUIElement::Release));
   m_children.clear();
}

///////////////////////////////////////

const char * cGUIPanelElement::GetId() const
{
   return m_id;
}

///////////////////////////////////////

void cGUIPanelElement::SetId(const char * pszId)
{
   m_id = pszId;
}

///////////////////////////////////////

bool cGUIPanelElement::HasFocus() const
{
   return false;
}

///////////////////////////////////////

void cGUIPanelElement::SetFocus(bool /*bFocus*/)
{
}

///////////////////////////////////////

bool cGUIPanelElement::IsVisible() const
{
   return m_bVisible;
}

///////////////////////////////////////

void cGUIPanelElement::SetVisible(bool bVisible)
{
   m_bVisible = bVisible;
}

///////////////////////////////////////

bool cGUIPanelElement::IsEnabled() const
{
   return m_bEnabled;
}

///////////////////////////////////////

void cGUIPanelElement::SetEnabled(bool bEnabled)
{
   m_bEnabled = bEnabled;
}

///////////////////////////////////////

tResult cGUIPanelElement::GetParent(IGUIElement * * ppParent)
{
   return m_pParent.GetPointer(ppParent);
}

///////////////////////////////////////

tResult cGUIPanelElement::SetParent(IGUIElement * pParent)
{
   SafeRelease(m_pParent);
   m_pParent = CTAddRef(pParent);
   return S_OK;
}

///////////////////////////////////////

tGUIPoint cGUIPanelElement::GetPosition() const
{
   return m_position;
}

///////////////////////////////////////

void cGUIPanelElement::SetPosition(const tGUIPoint & point)
{
   m_position = point;
}

///////////////////////////////////////

tGUISize cGUIPanelElement::GetSize() const
{
   return m_size;
}

///////////////////////////////////////

void cGUIPanelElement::SetSize(const tGUISize & size)
{
   m_size = size;
}

///////////////////////////////////////

bool cGUIPanelElement::Contains(const tGUIPoint & point)
{
   return false; // TODO
}

///////////////////////////////////////

tResult cGUIPanelElement::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}

///////////////////////////////////////

tResult cGUIPanelElement::GetRendererClass(tGUIString * pRendererClass)
{
   if (pRendererClass == NULL)
      return E_POINTER;
   *pRendererClass = "panel";
   return S_OK;
}

///////////////////////////////////////

tResult cGUIPanelElement::GetRenderer(IGUIElementRenderer * * ppRenderer)
{
   return m_pRenderer.GetPointer(ppRenderer);
}

///////////////////////////////////////

tResult cGUIPanelElement::SetRenderer(IGUIElementRenderer * pRenderer)
{
   SafeRelease(m_pRenderer);
   m_pRenderer = CTAddRef(pRenderer);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIPanelElement::AddElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (CTIsSameObject(*iter, pElement))
      {
         return S_FALSE;
      }
   }

   m_children.push_back(CTAddRef(pElement));
   return S_OK;
}

///////////////////////////////////////

tResult cGUIPanelElement::RemoveElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (CTIsSameObject(*iter, pElement))
      {
         m_children.erase(iter);
         pElement->Release();
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIPanelElement::GetElements(IGUIElementEnum * * ppElements)
{
   return GUIElementEnumCreate(m_children, ppElements);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(panel, cGUIPanelElementFactory);

tResult cGUIPanelElementFactory::CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      cAutoIPtr<IGUIPanelElement> pPanel = static_cast<IGUIPanelElement *>(new cGUIPanelElement);
      if (!!pPanel)
      {
         // TODO

         *ppElement = CTAddRef(pPanel);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUIPanelElement *>(new cGUIPanelElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelRenderer
//

///////////////////////////////////////

cGUIPanelRenderer::cGUIPanelRenderer()
 : m_pFont(FontCreateDefault())
{
}

///////////////////////////////////////

cGUIPanelRenderer::~cGUIPanelRenderer()
{
}

///////////////////////////////////////

tResult cGUIPanelRenderer::Render(IGUIElement * pElement, IRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIPanelElement> pPanel;
   if (pElement->QueryInterface(IID_IGUIPanelElement, (void**)&pPanel) == S_OK)
   {
      tGUIPoint pos = pPanel->GetPosition();
      tGUISize size = pPanel->GetSize();

      // TODO: render background, if any

      tResult result = S_OK;

      cAutoIPtr<IGUIElementEnum> pEnum;
      if (pPanel->GetElements(&pEnum) == S_OK)
      {
         cAutoIPtr<IGUIElement> pChild;
         ulong count = 0;

         while ((pEnum->Next(1, &pChild, &count) == S_OK) && (count == 1))
         {
            cAutoIPtr<IGUIElementRenderer> pChildRenderer;
            if (pChild->GetRenderer(&pChildRenderer) == S_OK)
            {
               if ((result = pChildRenderer->Render(pChild, pRenderDevice)) != S_OK)
               {
                  break;
               }
            }

            SafeRelease(pChild);
            count = 0;
         }
      }

      return result;
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUIPanelRenderer::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      cAutoIPtr<IGUIPanelElement> pPanel;
      if (pElement->QueryInterface(IID_IGUIPanelElement, (void**)&pPanel) == S_OK)
      {
         // TODO
      }
   }
   return tGUISize(0,0);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(panel, cGUIPanelRendererFactory);

tResult cGUIPanelRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   *ppRenderer = static_cast<IGUIElementRenderer *>(new cGUIPanelRenderer);
   return (*ppRenderer != NULL) ? S_OK : E_OUTOFMEMORY;
}

///////////////////////////////////////////////////////////////////////////////
