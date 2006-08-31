///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifontfactory.h"

#include "configapi.h"

#if HAVE_DIRECTX
#define WIN32_LEAN_AND_MEAN
#include <d3dx9.h>
#pragma comment(lib, "dxguid")
#endif

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

extern tResult GUIFontCreateFreetype(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);

// defined in guifontw32.cpp for Windows, or guifontx11.cpp for Linux
extern tResult GUIFontCreateGL(const tChar * pszFontName, int pointSize, uint effects, IGUIFont * * ppFont);

#if HAVE_DIRECTX
extern tResult GUIFontCreateD3D(IDirect3DDevice9 * pD3dDevice, const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);
#endif

tResult GUIFontCreate(const cGUIFontDesc & fontDesc, IUnknown * pUnk, IGUIFont * * ppFont)
{
   //static cGUIFontDesc defaultFontDesc;
   //static bool bFirstCall = true;
   //if (bFirstCall)
   //{
   //   bFirstCall = false;
   //   GUIFontDescDefault(&defaultFontDesc);
   //}

   UseGlobal(GUIFontCache);

   if (pGUIFontCache->GetFont(fontDesc, ppFont) == S_OK)
   {
      return S_OK;
   }

   cAutoIPtr<IGUIFont> pFont;

#if HAVE_DIRECTX
   if (pUnk != NULL)
   {
      cAutoIPtr<IDirect3DDevice9> pD3dDevice;
      if (pUnk->QueryInterface(IID_IDirect3DDevice9, (void**)&pD3dDevice) != S_OK)
      {
         ErrorMsg("Must pass an IDirect3DDevice* interface pointer to create a D3D font\n");
         return E_FAIL;
      }
      if (GUIFontCreateD3D(pD3dDevice, fontDesc, &pFont) == S_OK)
      {
         pGUIFontCache->SetFont(fontDesc, pFont);
         return pFont.GetPointer(ppFont);
      }
      return E_FAIL;
   }
#endif

   if (GUIFontCreateFreetype(fontDesc, &pFont) == S_OK)
   {
      pGUIFontCache->SetFont(fontDesc, pFont);
      return pFont.GetPointer(ppFont);
   }

   // TODO: convert to points
   if (fontDesc.GetSizeType() != kGUIFontSizePoints)
   {
      return E_FAIL;
   }

   if (GUIFontCreateGL(fontDesc.GetFace(), fontDesc.GetSize(), fontDesc.GetEffects(), &pFont) == S_OK)
   {
      pGUIFontCache->SetFont(fontDesc, pFont);
      return pFont.GetPointer(ppFont);
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIFontDescDefault(cGUIFontDesc * pFontDesc)
{
   if (pFontDesc == NULL)
   {
      return E_POINTER;
   }

   tChar szTypeFace[32];
   memset(szTypeFace, 0, sizeof(szTypeFace));
   if (ConfigGetString("default_font_win32", szTypeFace, _countof(szTypeFace)) != S_OK)
   {
      ConfigGetString("default_font", szTypeFace, _countof(szTypeFace));
   }

   int pointSize = 10;
   if (ConfigGet("default_font_size_win32", &pointSize) != S_OK)
   {
      ConfigGet("default_font_size", &pointSize);
   }

   int effects = kGFE_None;
   if (ConfigGet("default_font_effects_win32", &effects) != S_OK)
   {
      ConfigGet("default_font_effects", &effects);
   }

   *pFontDesc = cGUIFontDesc(szTypeFace, pointSize, kGUIFontSizePoints, effects);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontCache
//

////////////////////////////////////////

cGUIFontCache::cGUIFontCache()
{
}

////////////////////////////////////////

cGUIFontCache::~cGUIFontCache()
{
}

////////////////////////////////////////

tResult cGUIFontCache::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cGUIFontCache::Term()
{
   tFontMap::iterator iter = m_fontMap.begin();
   for (; iter != m_fontMap.end(); iter++)
   {
      SafeRelease(iter->second);
   }
   m_fontMap.clear();
   return S_OK;
}

////////////////////////////////////////

tResult cGUIFontCache::GetFont(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont)
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
   else
   {
      return S_FALSE;
   }
}

////////////////////////////////////////

tResult cGUIFontCache::SetFont(const cGUIFontDesc & fontDesc, IGUIFont * pFont)
{
   tFontMap::iterator f = m_fontMap.find(fontDesc);
   if (f != m_fontMap.end())
   {
      if (CTIsSameObject(f->second, pFont))
      {
         WarnMsg("Multiple calls to IGUIFontCache::SetFont with the same font\n");
         return S_FALSE;
      }
      SafeRelease(f->second);
      m_fontMap.erase(f);
   }

   if (pFont != NULL)
   {
      m_fontMap[fontDesc] = CTAddRef(pFont);
   }

   return S_OK;
}

///////////////////////////////////////

tResult GUIFontCacheCreate()
{
   cAutoIPtr<IGUIFontCache> p(new cGUIFontCache);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IGUIFontCache, static_cast<IGUIFontCache*>(p));
}

///////////////////////////////////////////////////////////////////////////////
