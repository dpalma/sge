///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifontfactory.h"

#include "dbgalloc.h" // must be last header


// defined in guifontw32.cpp for Windows, or guifontx11.cpp for Linux
extern tResult GUIFontCreateGL(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);

#if HAVE_DIRECTX
extern tResult GUIFontCreateD3D(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontFactory
//

////////////////////////////////////////

cGUIFontFactory::cGUIFontFactory()
{
   RegisterGlobalObject(IID_IGUIFontFactory, static_cast<IGlobalObject*>(this));
}

////////////////////////////////////////

cGUIFontFactory::~cGUIFontFactory()
{
}

////////////////////////////////////////

tResult cGUIFontFactory::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cGUIFontFactory::Term()
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

tResult cGUIFontFactory::CreateFontA(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont)
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

#if HAVE_DIRECTX
   if (GUIFontCreateD3D(fontDesc, &pFont) == S_OK)
   {
      m_fontMap[fontDesc] = CTAddRef(pFont);
      *ppFont = CTAddRef(pFont);
      return S_OK;
   }
#endif

   return E_FAIL;
}

////////////////////////////////////////

tResult cGUIFontFactory::CreateFontW(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont)
{
   return CreateFontA(fontDesc, ppFont);
}

///////////////////////////////////////

void GUIFontFactoryCreate()
{
   cAutoIPtr<IGUIFontFactory>(new cGUIFontFactory);
}

///////////////////////////////////////////////////////////////////////////////
