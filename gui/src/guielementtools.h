///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTTOOLS_H
#define INCLUDED_GUIELEMENTTOOLS_H

#include "gui/guielementapi.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);

F_DECLARE_INTERFACE(IGUIStyle);


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

tGUIString GUIElementType(IUnknown * pUnkElement);

bool GUIElementIdMatch(IGUIElement * pElement, const tChar * pszId);

tResult GUIStyleWidth(IGUIStyle * pStyle, tGUISizeType baseWidth, tGUISizeType * pWidth);
tResult GUIStyleHeight(IGUIStyle * pStyle, tGUISizeType baseHeight, tGUISizeType * pHeight);

void GUIPlaceElement(const tGUIRect & field, IGUIElement * pGUIElement);

tGUIPoint GUIElementAbsolutePosition(IGUIElement * pGUIElement, uint * pnParents = NULL);

tResult GUIElementFont(IGUIElement * pElement, IRenderFont * * ppFont);

tResult GUISetText(const tGUIChar * pszText, tGUIString * pString);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTTOOLS_H
