///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipanel.h"
#include "guielementbasetem.h"
#include "guielementenum.h"

#include "font.h"
#include "color.h"
#include "render.h"

#include "globalobj.h"

#include <tinyxml.h>
#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelElement
//

///////////////////////////////////////

cGUIPanelElement::cGUIPanelElement()
{
}

///////////////////////////////////////

cGUIPanelElement::~cGUIPanelElement()
{
   std::for_each(m_children.begin(), m_children.end(), CTInterfaceMethodRef(&IGUIElement::Release));
   m_children.clear();
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
         tResult result = S_OK;

         UseGlobal(GUIFactory);

         for (TiXmlElement * pXmlChild = pXmlElement->FirstChildElement(); 
              pXmlChild != NULL; pXmlChild = pXmlChild->NextSiblingElement())
         {
            if (pXmlChild->Type() == TiXmlNode::ELEMENT)
            {
               cAutoIPtr<IGUIElement> pChildElement;
               if (pGUIFactory->CreateElement(pXmlChild->Value(), pXmlChild, &pChildElement) == S_OK)
               {
                  if ((result = pPanel->AddElement(pChildElement)) != S_OK)
                  {
                     DebugMsg("WARNING: Error creating child element of panel\n");
                     break;
                  }
               }
            }
         }

         if (result == S_OK)
         {
            *ppElement = CTAddRef(pPanel);
         }

         return result;
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
