///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilabel.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElement
//

///////////////////////////////////////

cGUILabelElement::cGUILabelElement()
 : m_id(""),
   m_bVisible(true),
   m_bEnabled(true),
   m_position(0,0),
   m_size(0,0),
   m_text("")
{
}

///////////////////////////////////////

cGUILabelElement::~cGUILabelElement()
{
}

///////////////////////////////////////

const char * cGUILabelElement::GetId() const
{
   return m_id;
}

///////////////////////////////////////

void cGUILabelElement::SetId(const char * pszId)
{
   m_id = pszId;
}

///////////////////////////////////////

bool cGUILabelElement::HasFocus() const
{
   return false;
}

///////////////////////////////////////

void cGUILabelElement::SetFocus(bool /*bFocus*/)
{
}

///////////////////////////////////////

bool cGUILabelElement::IsVisible() const
{
   return m_bVisible;
}

///////////////////////////////////////

void cGUILabelElement::SetVisible(bool bVisible)
{
   m_bVisible = bVisible;
}

///////////////////////////////////////

bool cGUILabelElement::IsEnabled() const
{
   return m_bEnabled;
}

///////////////////////////////////////

void cGUILabelElement::SetEnabled(bool bEnabled)
{
   m_bEnabled = bEnabled;
}

///////////////////////////////////////

tResult cGUILabelElement::GetParent(IGUIElement * * ppParent)
{
   return m_pParent.GetPointer(ppParent);
}

///////////////////////////////////////

tResult cGUILabelElement::SetParent(IGUIElement * pParent)
{
   SafeRelease(m_pParent);
   m_pParent = CTAddRef(pParent);
   return S_OK;
}

///////////////////////////////////////

tGUIPoint cGUILabelElement::GetPosition() const
{
   return m_position;
}

///////////////////////////////////////

void cGUILabelElement::SetPosition(const tGUIPoint & point)
{
   m_position = point;
}

///////////////////////////////////////

tGUISize cGUILabelElement::GetSize() const
{
   return m_size;
}

///////////////////////////////////////

void cGUILabelElement::SetSize(const tGUISize & size)
{
   m_size = size;
}

///////////////////////////////////////

bool cGUILabelElement::Contains(const tGUIPoint & point)
{
   return false; // TODO
}

///////////////////////////////////////

const char * cGUILabelElement::GetText() const
{
   return m_text;
}

///////////////////////////////////////

void cGUILabelElement::SetText(const char * pszText)
{
   m_text = pszText;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(label, cGUILabelElementFactory);

tResult cGUILabelElementFactory::CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      cAutoIPtr<IGUILabelElement> pLabel = static_cast<IGUILabelElement *>(new cGUILabelElement);
      if (!!pLabel)
      {
         if (pXmlElement->Attribute("text"))
         {
            pLabel->SetText(pXmlElement->Attribute("text"));
         }

         *ppElement = CTAddRef(pLabel);
         return S_OK;
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
