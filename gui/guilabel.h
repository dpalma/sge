///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUILABEL_H
#define INCLUDED_GUILABEL_H

#include "guielementbase.h"

#include "script/scriptapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElement
//

class cGUILabelElement : public cComObject2<cGUIElementBase<IGUILabelElement>,
                                            &IID_IGUILabelElement,
                                            IMPLEMENTS(IScriptable)>
{
public:
   cGUILabelElement();
   ~cGUILabelElement();

   // IGUILabelElement methods
   virtual const tGUIChar * GetText() const;
   virtual tResult GetText(tGUIString * pText);
   virtual tResult SetText(const tGUIChar * pszText);

   // IScriptable methods
   virtual tResult Invoke(const char * pszMethodName,
                          int argc, const tScriptVar * argv,
                          int nMaxResults, tScriptVar * pResults);

private:
   tGUIString m_text;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUILABEL_H
