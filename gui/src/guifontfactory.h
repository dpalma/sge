///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFONTFACTORY_H
#define INCLUDED_GUIFONTFACTORY_H

#include "guiapi.h"

#include "globalobjdef.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontCache
//

class cGUIFontCache : public cComObject2<IMPLEMENTS(IGUIFontCache), IMPLEMENTS(IGlobalObject)>
{
   cGUIFontCache(const cGUIFontCache &); // un-implemented
   void operator=(const cGUIFontCache &); // un-implemented

public:
   cGUIFontCache();
   ~cGUIFontCache();

   DECLARE_NAME(GUIFontCache)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult GetFont(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);
   virtual tResult SetFont(const cGUIFontDesc & fontDesc, IGUIFont * pFont);

private:
   typedef std::map<cGUIFontDesc, IGUIFont*, std::less<cGUIFontDesc> > tFontMap;
   tFontMap m_fontMap;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFONTFACTORY_H
