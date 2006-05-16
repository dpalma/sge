///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifontfactory.h"

#if HAVE_DIRECTX
#include <d3dx9.h>
#endif

#include "dbgalloc.h" // must be last header


extern tResult GUIFontCreateFreetype(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);

// defined in guifontw32.cpp for Windows, or guifontx11.cpp for Linux
extern tResult GUIFontCreateGL(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);

#if HAVE_DIRECTX
extern tResult GUIFontCreateD3D(IDirect3DDevice9 * pD3dDevice, const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontFactory
//

////////////////////////////////////////

cGUIFontFactory::cGUIFontFactory()
{
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

   if (GUIFontCreateFreetype(fontDesc, &pFont) == S_OK)
   {
      m_fontMap[fontDesc] = CTAddRef(pFont);
      *ppFont = CTAddRef(pFont);
      return S_OK;
   }

#if HAVE_DIRECTX
   if (m_pD3dDevice != NULL)
   {
      if (GUIFontCreateD3D(m_pD3dDevice, fontDesc, &pFont) == S_OK)
      {
         m_fontMap[fontDesc] = CTAddRef(pFont);
         *ppFont = CTAddRef(pFont);
         return S_OK;
      }
   }
#endif

   if (GUIFontCreateGL(fontDesc, &pFont) == S_OK)
   {
      m_fontMap[fontDesc] = CTAddRef(pFont);
      *ppFont = CTAddRef(pFont);
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cGUIFontFactory::CreateFontW(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont)
{
   return CreateFontA(fontDesc, ppFont);
}

////////////////////////////////////////

#if HAVE_DIRECTX
tResult cGUIFontFactory::SetDirect3DDevice(IDirect3DDevice9 * pD3dDevice)
{
#if HAVE_DIRECTX
   SafeRelease(m_pD3dDevice);
   m_pD3dDevice = CTAddRef(pD3dDevice);
   return S_OK;
#else
   return E_NOTIMPL;
#endif
}
#endif

///////////////////////////////////////

tResult GUIFontFactoryCreate()
{
   cAutoIPtr<IGUIFontFactory> p(new cGUIFontFactory);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IGUIFontFactory, static_cast<IGUIFontFactory*>(p));
}

///////////////////////////////////////////////////////////////////////////////
