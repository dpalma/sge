///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTTOOLS_H
#define INCLUDED_GUIELEMENTTOOLS_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

void GUISizeElement(const tGUIRect & field, IGUIElement * pGUIElement);

void GUIPlaceElement(const tGUIRect & field, IGUIElement * pGUIElement);

tGUIPoint GUIElementAbsolutePosition(IGUIElement * pGUIElement);

tResult GUIElementStandardAttributes(const TiXmlElement * pXmlElement, 
                                     IGUIElement * pGUIElement);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTTOOLS_H
