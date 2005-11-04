///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTTOOLS_H
#define INCLUDED_GUIELEMENTTOOLS_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSameAs
//

class cSameAs
{
public:
   cSameAs(IUnknown * pUnknown) : m_pUnknown(CTAddRef(pUnknown))
   {
   }

   bool operator()(IUnknown * pUnknown2)
   {
      return CTIsSameObject(m_pUnknown, pUnknown2);
   }

private:
   cAutoIPtr<IUnknown> m_pUnknown;
};

///////////////////////////////////////////////////////////////////////////////

bool IsDescendant(IGUIElement * pParent, IGUIElement * pElement);

tResult GUIElementType(IUnknown * pUnkElement, cStr * pType);

bool GUIElementIdMatch(IGUIElement * pElement, const tChar * pszId);

tResult GUIElementRenderChildren(IGUIElement * pParent, IGUIRenderDevice * pRenderDevice);

tResult GUISizeElement(IGUIElement * pElement, const tGUISize & relativeTo);

void GUIPlaceElement(const tGUIRect & field, IGUIElement * pGUIElement);

tGUIPoint GUIElementAbsolutePosition(IGUIElement * pGUIElement, uint * pnParents = NULL);

tResult GUIElementStandardAttributes(const TiXmlElement * pXmlElement, 
                                     IGUIElement * pGUIElement);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSizeAndPlaceElement
//

class cSizeAndPlaceElement
{
public:
   cSizeAndPlaceElement(const tGUIRect & rect);

   tResult operator()(IGUIElement * pElement);

private:
   const tGUIRect m_rect;
   const tGUISize m_size;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderElement
//

class cRenderElement
{
   void operator =(const cRenderElement &);

public:
   cRenderElement(IGUIRenderDevice * pRenderDevice);
   cRenderElement(const cRenderElement & other);
   ~cRenderElement();

   tResult operator()(IGUIElement * pGUIElement);

private:
   cAutoIPtr<IGUIRenderDevice> m_pRenderDevice;
};


///////////////////////////////////////////////////////////////////////////////

template <typename F>
ulong ForEach(IGUIElementEnum * pEnum, F f)
{
   IGUIElement * pElements[32];
   ulong count = 0, total = 0;
   while (SUCCEEDED((pEnum->Next(_countof(pElements), &pElements[0], &count))) && (count > 0))
   {
      for (ulong i = 0; i < count; i++)
      {
         f(pElements[i]);
         SafeRelease(pElements[i]);
      }
      total += count;
      count = 0;
   }
   return total;
}


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTTOOLS_H
