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

bool GUIElementIdMatch(IGUIElement * pElement, const tChar * pszId);

tResult GUIElementCreateChildren(const TiXmlElement * pXmlElement, 
                                 IGUIContainerElement * pContainer);

tResult GUIElementRenderChildren(IGUIContainerElement * pContainer, IGUIRenderDevice * pRenderDevice);

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

#endif // !INCLUDED_GUIELEMENTTOOLS_H
