///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifontfactoryd3d.h"

#include <d3d9.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

extern tResult GUIFontCreateD3D(IDirect3DDevice9 * pD3dDevice,
                                const cGUIFontDesc & fontDesc,
                                IGUIFont * * ppFont);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontFactoryD3D
//

////////////////////////////////////////

cGUIFontFactoryD3D::cGUIFontFactoryD3D()
{
   RegisterGlobalObject(IID_IGUIFontFactory, static_cast<IGlobalObject*>(this));
}

////////////////////////////////////////

cGUIFontFactoryD3D::~cGUIFontFactoryD3D()
{
}

////////////////////////////////////////

tResult cGUIFontFactoryD3D::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cGUIFontFactoryD3D::Term()
{
   ReleaseAllCachedFonts();

   SafeRelease(m_pD3dDevice);

   return S_OK;
}

////////////////////////////////////////

tResult cGUIFontFactoryD3D::CreateFont2(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont)
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
   if (GUIFontCreateD3D(m_pD3dDevice, fontDesc, &pFont) == S_OK)
   {
      m_fontMap[fontDesc] = CTAddRef(pFont);
      *ppFont = CTAddRef(pFont);
      return S_OK;
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cGUIFontFactoryD3D::SetD3DDevice(IDirect3DDevice9 * pDevice)
{
   SafeRelease(m_pD3dDevice);
   ReleaseAllCachedFonts();
   m_pD3dDevice = CTAddRef(pDevice);
   return S_OK;
}

////////////////////////////////////////

void cGUIFontFactoryD3D::ReleaseAllCachedFonts()
{
   tFontMap::iterator iter = m_fontMap.begin();
   tFontMap::iterator end = m_fontMap.end();
   for (; iter != end; iter++)
   {
      SafeRelease(iter->second);
   }
   m_fontMap.clear();
}

///////////////////////////////////////

void GUIFontFactoryCreateD3D()
{
   cAutoIPtr<IGUIFontFactory>(new cGUIFontFactoryD3D);
}

///////////////////////////////////////////////////////////////////////////////
