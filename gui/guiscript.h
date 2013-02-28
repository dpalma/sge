///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUISCRIPT_H
#define INCLUDED_GUISCRIPT_H

#include "guielementbase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIScriptElement
//

class cGUIScriptElement : public cComObject<cGUIElementBase<IGUIScriptElement>, &IID_IGUIScriptElement>
{
public:
   cGUIScriptElement();
   ~cGUIScriptElement();

   virtual tResult GetScript(tGUIString * pScript);
   virtual tResult SetScript(const tChar * pszScript);

   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer);

private:
   tGUIString m_script;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUISCRIPT_H
