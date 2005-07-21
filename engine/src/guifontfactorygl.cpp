///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifontfactorygl.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header


// defined in guifontw32.cpp for Windows, or guifontx11.cpp for Linux
extern tResult GUIFontCreateGL(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontFactoryGL
//

////////////////////////////////////////

cGUIFontFactoryGL::cGUIFontFactoryGL()
{
   RegisterGlobalObject(IID_IGUIFontFactory, static_cast<IGlobalObject*>(this));
}

////////////////////////////////////////

cGUIFontFactoryGL::~cGUIFontFactoryGL()
{
}

////////////////////////////////////////

tResult cGUIFontFactoryGL::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cGUIFontFactoryGL::Term()
{
   tFontMap::iterator iter = m_fontMap.begin();
   tFontMap::iterator end = m_fontMap.end();
   for (; iter != end; iter++)
   {
      SafeRelease(iter->second);
   }
   m_fontMap.clear();
   return S_OK;
}

////////////////////////////////////////

tResult cGUIFontFactoryGL::CreateFont2(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont)
{
   if (ppFont == NULL)
   {
      return E_POINTER;
   }

   tFontMap::iterator f = m_fontMap.find(fontDesc);
   if (f != m_fontMap.end())
   {
      *ppFont = CTAddRef(f->second);
      return S_OK;
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GUIFontCreateGL(fontDesc, &pFont) == S_OK)
   {
      m_fontMap[fontDesc] = CTAddRef(pFont);
      *ppFont = CTAddRef(pFont);
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

void GUIFontFactoryCreateGL()
{
   cAutoIPtr<IGUIFontFactory>(new cGUIFontFactoryGL);
}

///////////////////////////////////////////////////////////////////////////////
