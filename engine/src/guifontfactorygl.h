///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFONTFACTORYGL_H
#define INCLUDED_GUIFONTFACTORYGL_H

#include "guiapi.h"

#include "globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontFactoryGL
//

class cGUIFontFactoryGL : public cComObject2<IMPLEMENTS(IGUIFontFactory), IMPLEMENTS(IGlobalObject)>
{
   cGUIFontFactoryGL(const cGUIFontFactoryGL &); // un-implemented
   void operator=(const cGUIFontFactoryGL &); // un-implemented

public:
   cGUIFontFactoryGL();
   ~cGUIFontFactoryGL();

   DECLARE_NAME(GUIFontFactory)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult CreateFont2(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);

private:
   typedef std::map<cGUIFontDesc, IGUIFont*> tFontMap;
   tFontMap m_fontMap;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFONTFACTORYGL_H
