///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilabel.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "globalobj.h"
#include "multivar.h"

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
   return m_text.empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cGUILabelElement::SetText(const char * pszText)
{
   if (pszText == NULL)
   {
      m_text.erase();
   }
   else
   {
      m_text = pszText;
   }
   return S_OK;
}

////////////////////////////////////////

tResult cGUILabelElement::Invoke(const char * pszMethodName,
                                 int argc, const tScriptVar * argv,
                                 int nMaxResults, tScriptVar * pResults)
{
   if (pszMethodName == NULL)
   {
      return E_POINTER;
   }

   if (strcmp(pszMethodName, "SetText") == 0)
   {
      if (argc == 1 && argv[0].IsString())
      {
         if (SetText(argv[0]) == S_OK)
         {
            return S_OK;
         }
      }
      else
      {
         return E_INVALIDARG;
      }
   }
   else if (strcmp(pszMethodName, "GetText") == 0)
   {
      tGUIString text;
      if (nMaxResults >= 1 && GetText(&text) == S_OK)
      {
         pResults[0] = text.c_str();
         return 1;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUILabelElementCreate(const TiXmlElement * pXmlElement,
                              IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementLabel) == 0)
      {
         cAutoIPtr<IGUILabelElement> pLabel = static_cast<IGUILabelElement *>(
             new cGUILabelElement);
         if (!!pLabel)
         {
            if (pXmlElement->Attribute(kAttribText))
            {
               pLabel->SetText(pXmlElement->Attribute(kAttribText));
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

AUTOREGISTER_GUIELEMENTFACTORYFN(label, GUILabelElementCreate);

///////////////////////////////////////////////////////////////////////////////
