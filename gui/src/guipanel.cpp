///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipanel.h"
#include "guielementbasetem.h"
#include "guicontainerbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

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

   cAutoIPtr<IGUILayoutManager> pLayout;
   if (GetLayout(&pLayout) == S_OK)
   {
      pLayout->Layout(this);
   }
   else
   {
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
}

///////////////////////////////////////

tResult cGUIPanelElement::OnEvent(IGUIEvent * pEvent)
{
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
      if (strcmp(pXmlElement->Value(), kElementPanel) == 0)
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
   }
   else
   {
      *ppElement = static_cast<IGUIPanelElement *>(new cGUIPanelElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
