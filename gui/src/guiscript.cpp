
// $Id$

#include "stdhdr.h"

#include "guiscript.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "globalobj.h"
#include "resourceapi.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIScriptElement
//

///////////////////////////////////////

cGUIScriptElement::cGUIScriptElement()
{
}

///////////////////////////////////////

cGUIScriptElement::~cGUIScriptElement()
{
}

///////////////////////////////////////

tResult cGUIScriptElement::GetScript(tGUIString * pScript)
{
   if (pScript == NULL)
   {
      return E_POINTER;
   }
   *pScript = m_script;
   return m_script.empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cGUIScriptElement::SetScript(const tChar * pszScript)
{
   if (pszScript == NULL)
   {
      m_script.erase();
   }
   else
   {
      m_script = pszScript;
   }
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScriptElement::GetRendererClass(tGUIString * pRendererClass)
{
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIScriptElement::GetRenderer(IGUIElementRenderer * * ppRenderer)
{
   return S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////

tResult GUIScriptElementCreate(const TiXmlElement * pXmlElement, IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementScript) == 0)
      {
         cAutoIPtr<IGUIScriptElement> pScript = static_cast<IGUIScriptElement *>(new cGUIScriptElement);
         if (!pScript)
         {
            return E_OUTOFMEMORY;
         }

         if (pXmlElement->Attribute(kAttribSrc) != NULL)
         {
            if (pXmlElement->FirstChild() != NULL)
            {
               ErrorMsg1("Script element should either specify an external script "
                  "file in the %s attribute, or include script code in the body of "
                  "the element, but not both\n", kAttribSrc);
               return E_FAIL;
            }

            UseGlobal(ResourceManager);
            const tChar * pszText = NULL;
            if (pResourceManager->Load(pXmlElement->Attribute(kAttribSrc), kRT_Text, NULL, (void**)&pszText) == S_OK)
            {
               pScript->SetScript(pszText);
            }
         }
         else
         {
            const TiXmlNode * pFirstChild = pXmlElement->FirstChild();
            if (pFirstChild != NULL)
            {
               const TiXmlText * pText = pFirstChild->ToText();
               if (pText != NULL)
               {
                  pScript->SetScript(pText->Value());
               }
            }
         }

         *ppElement = CTAddRef(pScript);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUIScriptElement *>(new cGUIScriptElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(script, GUIScriptElementCreate);

///////////////////////////////////////////////////////////////////////////////
