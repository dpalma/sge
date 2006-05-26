///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilabel.h"
#include "guielementbasetem.h"
#include "guistrings.h"

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

const tGUIChar * cGUILabelElement::GetText() const
{
   return m_text.c_str();
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

tResult cGUILabelElement::SetText(const tGUIChar * pszText)
{
   if (pszText == NULL)
   {
      m_text.erase();
   }
   else
   {
      m_text.assign(pszText);
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
      if (argc == 1)
      {
         if (argv[0].IsString())
         {
            SetText(argv[0]);
         }
         else if (argv[0].IsEmpty())
         {
            SetText(NULL);
         }
         else
         {
            return E_INVALIDARG;
         }
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

   return 0;
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
         if (!pLabel)
         {
            return E_OUTOFMEMORY;
         }

         pLabel->SetText(pXmlElement->Attribute(kAttribText));

         *ppElement = CTAddRef(pLabel);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUILabelElement *>(new cGUILabelElement);
      return (*ppElement != NULL) ? S_OK : E_OUTOFMEMORY;
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(label, GUILabelElementCreate);

///////////////////////////////////////////////////////////////////////////////
