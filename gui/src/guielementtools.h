///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTTOOLS_H
#define INCLUDED_GUIELEMENTTOOLS_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

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

tResult GUIElementRenderChildren(IGUIContainerElement * pContainer);

tResult GUIElementSizeFromStyle(IGUIElement * pElement,
                                const tGUISize & relativeTo,
                                tGUISize * pSize);

void GUISizeElement(const tGUIRect & field, IGUIElement * pGUIElement);

void GUIPlaceElement(const tGUIRect & field, IGUIElement * pGUIElement);

tGUIPoint GUIElementAbsolutePosition(IGUIElement * pGUIElement, uint * pnParents = NULL);

tResult GUIElementStandardAttributes(const TiXmlElement * pXmlElement, 
                                     IGUIElement * pGUIElement);

class cSizeAndPlaceElement
{
public:
   cSizeAndPlaceElement(const tGUIRect & rect) : m_rect(rect) {}

   tResult operator()(IGUIElement * pGUIElement)
   {
      Assert(pGUIElement != NULL);
      GUISizeElement(m_rect, pGUIElement);
      GUIPlaceElement(m_rect, pGUIElement);
      return S_OK;
   }

private:
   tGUIRect m_rect;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTTOOLS_H
