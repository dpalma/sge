///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"

#include "sceneapi.h"

#include "keys.h"
#include "resmgr.h"
#include "readwriteapi.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

BEGIN_CONSTRAINTS()
   AFTER_GUID(IID_IScene)
END_CONSTRAINTS()

///////////////////////////////////////

cGUIContext::cGUIContext()
 : cGlobalObject<IMPLEMENTSCP(IGUIContext, IGUIEventListener)>("GUIContext", CONSTRAINTS())
{
   UseGlobal(Scene);
   pScene->AddInputListener(kSL_InGameUI, &m_inputListener);
}

///////////////////////////////////////

cGUIContext::~cGUIContext()
{
   UseGlobal(Scene);
   pScene->RemoveInputListener(kSL_InGameUI, &m_inputListener);
}

///////////////////////////////////////

tResult cGUIContext::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::Term()
{
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethodRef(&IGUIElement::Release));
   m_elements.clear();
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::AddEventListener(IGUIEventListener * pListener)
{
   return Connect(pListener);
}

///////////////////////////////////////

tResult cGUIContext::RemoveEventListener(IGUIEventListener * pListener)
{
   return Disconnect(pListener);
}

///////////////////////////////////////

tResult cGUIContext::GetFocus(IGUIElement * * ppElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIContext::SetFocus(IGUIElement * pElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIContext::GetCapture(IGUIElement * * ppElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIContext::SetCapture(IGUIElement * pElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIContext::LoadFromResource(const char * psz)
{
   UseGlobal(ResourceManager);
   cAutoIPtr<IReader> pReader = pResourceManager->Find(psz);
   if (!pReader)
      return E_FAIL;

   pReader->Seek(0, kSO_End);
   int len = pReader->Tell();
   pReader->Seek(0, kSO_Set);

   char * pszContents = new char[len + 1];
   if (pszContents == NULL)
      return E_OUTOFMEMORY;

   if (pReader->Read(pszContents, len) != S_OK)
   {
      delete [] pszContents;
      return E_FAIL;
   }

   pszContents[len] = 0;

   tResult result = LoadFromString(pszContents);

   delete [] pszContents;

   return result;
}

///////////////////////////////////////

tResult cGUIContext::LoadFromString(const char * psz)
{
   TiXmlDocument doc;
   doc.Parse(psz);

   if (doc.Error())
   {
      DebugMsg1("TiXml parse error: %s\n", doc.ErrorDesc());
      return E_FAIL;
   }

   UseGlobal(GUIFactory);

   ulong nElementsCreated = 0;

   TiXmlElement * pXmlElement;
   for (pXmlElement = doc.FirstChildElement(); pXmlElement != NULL; pXmlElement = pXmlElement->NextSiblingElement())
   {
      if (pXmlElement->Type() == TiXmlNode::ELEMENT)
      {
         cAutoIPtr<IGUIElement> pGUIElement;
         if (pGUIFactory->CreateElement(pXmlElement->Value(), pXmlElement, &pGUIElement) == S_OK)
         {
            nElementsCreated++;
            m_elements.push_back(CTAddRef(pGUIElement));
         }
      }
   }

   return nElementsCreated;
}

///////////////////////////////////////

bool cGUIContext::HandleInputEvent(const sInputEvent * pEvent)
{
   if (KeyIsMouse(pEvent->key))
   {
   }
   else
   {
   }

   return false;
}

///////////////////////////////////////

bool cGUIContext::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   cGUIContext * pOuter = CTGetOuter(cGUIContext, m_inputListener);
   return pOuter->HandleInputEvent(pEvent);
}

///////////////////////////////////////

void GUIContextCreate()
{
   cAutoIPtr<IGUIContext>(new cGUIContext);
}

///////////////////////////////////////////////////////////////////////////////
