///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guielementenum.h"

#include <algorithm>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIElementEnum
//

///////////////////////////////////////

tResult GUIElementEnumCreate(const tGUIElementList & elements, IGUIElementEnum * * ppEnum)
{
   if (ppEnum == NULL)
      return E_POINTER;
   *ppEnum = static_cast<IGUIElementEnum *>(new cGUIElementEnum(elements));
   if ((*ppEnum) == NULL)
      return E_OUTOFMEMORY;
   return S_OK;
}

///////////////////////////////////////

cGUIElementEnum::cGUIElementEnum(const tGUIElementList & elements)
 : m_elements(elements.size())
{
   std::copy(elements.begin(), elements.end(), m_elements.begin());
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethod(&IGUIElement::AddRef));
   m_iterator = m_elements.begin();
}

///////////////////////////////////////

cGUIElementEnum::~cGUIElementEnum()
{
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethod(&IGUIElement::Release));
   m_elements.clear();
   m_iterator = m_elements.end();
}

///////////////////////////////////////

tResult cGUIElementEnum::Next(ulong count, IGUIElement * * ppElements, ulong * pnElements)
{
   Assert(ppElements != NULL && pnElements != NULL);

   ulong nReturned = 0;

   for (ulong i = 0; i < count; i++)
   {
      if (m_iterator == m_elements.end())
      {
         break;
      }
      else
      {
         *ppElements = CTAddRef(*m_iterator);
         nReturned++;
         m_iterator++;
         ppElements++;
      }
   }

   if (pnElements != NULL)
   {
      *pnElements = nReturned;
   }

   return (nReturned == count) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cGUIElementEnum::Skip(ulong count)
{
   uint nSkipped = 0;
   while (count-- && (m_iterator != m_elements.end()))
   {
      nSkipped++;
      m_iterator++;
   }
   return (nSkipped == count) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cGUIElementEnum::Reset()
{
   m_iterator = m_elements.begin();
   return S_OK;
}

///////////////////////////////////////

tResult cGUIElementEnum::Clone(IGUIElementEnum * * ppEnum)
{
   Assert(ppEnum != NULL);
   *ppEnum = static_cast<IGUIElementEnum *>(new cGUIElementEnum(m_elements));
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
