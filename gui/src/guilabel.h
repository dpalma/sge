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

#endif // !INCLUDED_GUILABEL_H
