///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipanel.h"
#include "guielementbasetem.h"
#include "guicontainerbasetem.h"
#include "guielementenum.h"
#include "guirender.h"
#include "guielementtools.h"

#include "font.h"
#include "color.h"
#include "render.h"

#include "globalobj.h"
#include "parse.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelElement
//

///////////////////////////////////////

cGUIPanelElement::cGUIPanelElement()
 : m_pInsets(NULL)
{
}

///////////////////////////////////////

cGUIPanelElement::~cGUIPanelElement()
{
   delete m_pInsets;
   m_pInsets = NULL;
}

///////////////////////////////////////

class cSizeAndPlaceElement
{
public:
   cSizeAndPlaceElement(const tGUIRect & rect) : m_rect(rect) {}

   void operator()(IGUIElement * pGUIElement)
   {
      Assert(pGUIElement != NULL);
      GUISizeElement(m_rect, pGUIElement);
      GUIPlaceElement(m_rect, pGUIElement);
   }

private:
   tGUIRect m_rect;
};

void cGUIPanelElement::SetSize(const tGUISize & size)
{
   tBaseClass::SetSize(size);

   tGUIRect rect(0, 0, Round(size.width), Round(size.height));

   tGUIInsets insets;
   if (GetInsets(&insets) == S_OK)
   {
      rect.left += insets.left;
      rect.top += insets.top;
      rect.right -= insets.right;
      rect.bottom -= insets.bottom;
   }

   ForEachElement(cSizeAndPlaceElement(rect));
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

tResult cGUIPanelElement::GetInsets(tGUIInsets * pInsets)
{
   if (pInsets == NULL)
   {
      return E_POINTER;
   }

   if (m_pInsets == NULL)
   {
      return S_FALSE;
   }

   *pInsets = *m_pInsets;

   return S_OK;
}

///////////////////////////////////////

tResult cGUIPanelElement::SetInsets(const tGUIInsets & insets)
{
   if (m_pInsets == NULL)
   {
      m_pInsets = new tGUIInsets;
      if (m_pInsets == NULL)
      {
         return E_OUTOFMEMORY;
      }
   }

   *m_pInsets = insets;

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(panel, cGUIPanelElementFactory);

tResult cGUIPanelElementFactory::CreateElement(const TiXmlElement * pXmlElement, 
                                               IGUIElement * * ppElement)
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

         GUIElementStandardAttributes(pXmlElement, pPanel);

         if (pXmlElement->Attribute("insets"))
         {
            double insetVals[4];
            if (ParseTuple(pXmlElement->Attribute("insets"), insetVals, _countof(insetVals)) == 4)
            {
               tGUIInsets insets;
               insets.left = Round(insetVals[0]);
               insets.top = Round(insetVals[1]);
               insets.right = Round(insetVals[2]);
               insets.bottom = Round(insetVals[3]);
               pPanel->SetInsets(insets);
            }
         }

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
      tGUIPoint pos = GUIElementAbsolutePosition(pPanel);
      tGUISize size = pPanel->GetSize();
      tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

      UseGlobal(GUIRenderingTools);

      // TODO HACK
      pGUIRenderingTools->Render3dRect(rect, 4, tGUIColor::Yellow, tGUIColor::Green, tGUIColor::Blue);

      tResult result = S_OK;

      cAutoIPtr<IGUIElementEnum> pEnum;
      if (pPanel->GetElements(&pEnum) == S_OK)
      {
         cAutoIPtr<IGUIElement> pChild;
         ulong count = 0;

         while ((pEnum->Next(1, &pChild, &count) == S_OK) && (count == 1))
         {
            if (pChild->IsVisible())
            {
               cAutoIPtr<IGUIElementRenderer> pChildRenderer;
               if (pChild->GetRenderer(&pChildRenderer) == S_OK)
               {
                  if ((result = pChildRenderer->Render(pChild, pRenderDevice)) != S_OK)
                  {
                     break;
                  }
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

tResult cGUIPanelRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, 
                                                 IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   *ppRenderer = static_cast<IGUIElementRenderer *>(new cGUIPanelRenderer);
   return (*ppRenderer != NULL) ? S_OK : E_OUTOFMEMORY;
}

///////////////////////////////////////////////////////////////////////////////
