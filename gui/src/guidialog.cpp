///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guidialog.h"
#include "guielementbasetem.h"
#include "guicontainerbasetem.h"
#include "guielementenum.h"
#include "guielementtools.h"

#include "font.h"
#include "color.h"
#include "render.h"

#include "globalobj.h"
#include "parse.h"

#include <tinyxml.h>
#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogElement
//

///////////////////////////////////////

cGUIDialogElement::cGUIDialogElement()
 : m_pInsets(NULL)
{
}

///////////////////////////////////////

cGUIDialogElement::~cGUIDialogElement()
{
   delete m_pInsets;
   m_pInsets = NULL;
}

///////////////////////////////////////

void cGUIDialogElement::SetSize(const tGUISize & size)
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

tResult cGUIDialogElement::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::GetRendererClass(tGUIString * pRendererClass)
{
   if (pRendererClass == NULL)
      return E_POINTER;
   *pRendererClass = "dialog";
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::GetInsets(tGUIInsets * pInsets)
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

tResult cGUIDialogElement::SetInsets(const tGUIInsets & insets)
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
// CLASS: cGUIDialogElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(dialog, cGUIDialogElementFactory);

tResult cGUIDialogElementFactory::CreateElement(const TiXmlElement * pXmlElement, 
                                                IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), "dialog") == 0)
      {
         cAutoIPtr<IGUIDialogElement> pDialog = static_cast<IGUIDialogElement *>(new cGUIDialogElement);
         if (!!pDialog)
         {
            tResult result = S_OK;

            GUIElementStandardAttributes(pXmlElement, pDialog);

            if (pXmlElement->Attribute("insets"))
            {
               double insetVals[4];
               if (ParseTuple(pXmlElement->Attribute("insets"), insetVals, _countof(insetVals)) == 4)
               {
                  tGUIInsets insets;
                  insets.left = insetVals[0];
                  insets.top = insetVals[1];
                  insets.right = insetVals[2];
                  insets.bottom = insetVals[3];
                  pDialog->SetInsets(insets);
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
                     if ((result = pDialog->AddElement(pChildElement)) != S_OK)
                     {
                        DebugMsg("WARNING: Error creating child element of panel\n");
                        break;
                     }
                  }
               }
            }

            if (result == S_OK)
            {
               *ppElement = CTAddRef(pDialog);
            }

            return result;
         }
      }
   }
   else
   {
      *ppElement = static_cast<IGUIDialogElement *>(new cGUIDialogElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogRenderer
//

///////////////////////////////////////

cGUIDialogRenderer::cGUIDialogRenderer()
{
}

///////////////////////////////////////

cGUIDialogRenderer::~cGUIDialogRenderer()
{
}

///////////////////////////////////////

tResult cGUIDialogRenderer::Render(IGUIElement * pElement, IRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIDialogElement> pDialog;
   if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialog) == S_OK)
   {
      tGUIPoint pos = GUIElementAbsolutePosition(pDialog);
      tGUISize size = pDialog->GetSize();

      UseGlobal(GUIRenderingTools);

      // TODO HACK
      pGUIRenderingTools->Render3dRect(
         tGUIRect(pos.x, pos.y, pos.x + size.width, pos.y + size.height), 
         4, tGUIColor::Yellow, tGUIColor::Green, tGUIColor::Blue);

      tResult result = S_OK;

      cAutoIPtr<IGUIElementEnum> pEnum;
      if (pDialog->GetElements(&pEnum) == S_OK)
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

tGUISize cGUIDialogRenderer::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      cAutoIPtr<IGUIDialogElement> pDialog;
      if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialog) == S_OK)
      {
         // TODO
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(dialog, cGUIDialogRendererFactory);

tResult cGUIDialogRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, 
                                                  IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   *ppRenderer = static_cast<IGUIElementRenderer *>(new cGUIDialogRenderer);
   return (*ppRenderer != NULL) ? S_OK : E_OUTOFMEMORY;
}

///////////////////////////////////////////////////////////////////////////////
