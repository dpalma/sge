///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipanel.h"
#include "guielementbasetem.h"
#include "guicontainerbasetem.h"
#include "guielementtools.h"

#include "font.h"
#include "color.h"
#include "renderapi.h"

#include "globalobj.h"

#include <tinyxml.h>

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
}

///////////////////////////////////////

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
         GUIElementStandardAttributes(pXmlElement, pPanel);

         if (GUIElementCreateChildren(pXmlElement, pPanel) == S_OK)
         {
            *ppElement = CTAddRef(pPanel);
            return S_OK;
         }
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

      if (GUIElementRenderChildren(pPanel, pRenderDevice) == S_OK)
      {
         return S_OK;
      }
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
