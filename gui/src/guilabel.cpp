///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilabel.h"
#include "guielementbasetem.h"
#include "guielementtools.h"

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
