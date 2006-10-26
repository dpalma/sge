///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipanel.h"
#include "guielementbasetem.h"
#include "guicontainerbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "tech/globalobj.h"

#include <tinyxml.h>

#include "tech/dbgalloc.h" // must be last header

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

tResult cGUIPanelElement::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIPanelCreate(IGUIPanelElement * * ppPanelElement)
{
   if (ppPanelElement == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IGUIPanelElement> pPanel(static_cast<IGUIPanelElement*>(new cGUIPanelElement));
   if (!pPanel)
   {
      return E_NOTIMPL;
   }
   return pPanel.GetPointer(ppPanelElement);
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIPanelElementCreate(const TiXmlElement * pXmlElement,
                              IGUIElement * pParent, IGUIElement * * ppElement)
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
         if (!pPanel)
         {
            return E_OUTOFMEMORY;
         }

         *ppElement = CTAddRef(pPanel);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUIPanelElement *>(new cGUIPanelElement);
      return (*ppElement != NULL) ? S_OK : E_OUTOFMEMORY;
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(panel, GUIPanelElementCreate);

///////////////////////////////////////////////////////////////////////////////
