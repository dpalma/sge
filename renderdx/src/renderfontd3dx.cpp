///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guifontd3d.h"

#ifndef _WIN32
#error ("This file is for Windows compilation only")
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if HAVE_DIRECTX
#include <d3dx9.h>
#endif

#include <cstring>

#include "dbgalloc.h" // must be last header

#if HAVE_DIRECTX
#ifdef _DEBUG
#pragma comment(lib, "d3dx9d")
#else
#pragma comment(lib, "d3dx9")
#endif
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontD3D
//

///////////////////////////////////////

cGUIFontD3D::cGUIFontD3D(ID3DXFont * pD3dxFont)
#if HAVE_DIRECTX
 : m_pD3dxFont(CTAddRef(pD3dxFont))
#endif
{
}

///////////////////////////////////////

cGUIFontD3D::~cGUIFontD3D()
{
}

///////////////////////////////////////

tResult cGUIFontD3D::RenderText(const char * pszText, int textLength, tRect * pRect,
                                uint flags, const cColor & color) const
{
#if HAVE_DIRECTX
   if (!!m_pD3dxFont)
   {
      RECT rect;
      rect.left = pRect->left;
      rect.top = pRect->top;
      rect.right = pRect->right;
      rect.bottom = pRect->bottom;

      DWORD format = 0;
      if (flags & kRT_Center) format |= DT_CENTER;
      if (flags & kRT_VCenter) format |= DT_VCENTER;
      if (flags & kRT_NoClip) format |= DT_NOCLIP;
      if (flags & kRT_CalcRect) format |= DT_CALCRECT;
      if (flags & kRT_SingleLine) format |= DT_SINGLELINE;
      if (flags & kRT_Bottom) format |= DT_BOTTOM;

      if (!m_pD3dxFont->DrawText(NULL, pszText, textLength, &rect, format, color.ToARGB8888()))
      {
         return E_FAIL;
      }

      if (flags & kRT_CalcRect)
      {
         pRect->left = rect.left;
         pRect->top = rect.top;
         pRect->right = rect.right;
         pRect->bottom = rect.bottom;
      }

      return S_OK;
   }
#endif // HAVE_DIRECTX

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIFontD3D::RenderText(const wchar_t * pszText, int textLength, tRect * pRect,
                                uint flags, const cColor & color) const
{
   return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////

#if HAVE_DIRECTX
tResult GUIFontCreateD3D(IDirect3DDevice9 * pD3dDevice, const cGUIFontDesc & fontDesc, IGUIFont * * ppFont)
{
   if (pD3dDevice == NULL || ppFont == NULL)
   {
      return E_POINTER;
   }

   HDC hScreenDC = GetDC(NULL);
   if (hScreenDC == NULL)
   {
      return E_FAIL;
   }

   int height = 0;
   if (fontDesc.GetSizeType() == kGUIFontSizePoints)
   {
      height = -MulDiv(fontDesc.GetSize(), GetDeviceCaps(hScreenDC, LOGPIXELSY), 72);
   }
   else if (fontDesc.GetSizeType() == kGUIFontSizeEm)
   {
      height = fontDesc.GetSize();
   }
   else
   {
      return E_NOTIMPL;
   }

   ReleaseDC(NULL, hScreenDC), hScreenDC = NULL;

   cAutoIPtr<ID3DXFont> pD3DXFont;
   if (D3DXCreateFont(pD3dDevice, height, 0, fontDesc.IsBold() ? FW_EXTRABOLD : FW_NORMAL,
      4, fontDesc.IsItalic(), DEFAULT_CHARSET, OUT_TT_PRECIS, PROOF_QUALITY,
      DEFAULT_PITCH | FF_DONTCARE, fontDesc.GetFace(), &pD3DXFont) == S_OK)
   {
      cAutoIPtr<cGUIFontD3D> pFont(new cGUIFontD3D(pD3DXFont));
      if (!pFont)
      {
         return E_OUTOFMEMORY;
      }

      *ppFont = CTAddRef(pFont);
      return S_OK;
   }

   return E_FAIL;
}
#endif // HAVE_DIRECTX

///////////////////////////////////////////////////////////////////////////////
