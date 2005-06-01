///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUILABEL_H
#define INCLUDED_GUILABEL_H

#include "guielementbase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElement
//

class cGUILabelElement : public cComObject<cGUIElementBase<IGUILabelElement>, &IID_IGUILabelElement>
{
public:
   cGUILabelElement();
   ~cGUILabelElement();

   virtual tResult GetText(tGUIString * pText);
   virtual tResult SetText(const char * pszText);

private:
   tGUIString m_text;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElementFactory
//

class cGUILabelElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUILABEL_H
