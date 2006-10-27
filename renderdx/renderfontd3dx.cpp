///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderfontd3dx.h"

#ifndef _WIN32
#error ("This file is for Windows compilation only")
#endif

#define WIN32_LEAN_AND_MEAN
#include <d3dx9.h>

#include <cstring>

#include "tech/dbgalloc.h" // must be last header

#ifdef _DEBUG
#pragma comment(lib, "d3dx9d")
#else
#pragma comment(lib, "d3dx9")
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderFontD3DX
//

///////////////////////////////////////

cRenderFontD3DX::cRenderFontD3DX(ID3DXFont * pD3dxFont)
 : m_pD3dxFont(CTAddRef(pD3dxFont))
{
}

///////////////////////////////////////

cRenderFontD3DX::~cRenderFontD3DX()
{
}

///////////////////////////////////////

tResult cRenderFontD3DX::RenderText(const tChar * pszText, int textLength, tRect * pRect,
                                    uint flags, const float color[4]) const
{
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

      D3DCOLOR color2 = 0;

      if (color != NULL)
      {
         byte r = static_cast<byte>(color[0] * 255);
         byte g = static_cast<byte>(color[1] * 255);
         byte b = static_cast<byte>(color[2] * 255);
         byte a = static_cast<byte>(color[3] * 255);
         color2 = D3DCOLOR_RGBA(r, g, b, a);
      }

      if (!m_pD3dxFont->DrawText(NULL, pszText, textLength, &rect, format, color2))
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

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult RenderFontCreateD3DX(const tChar * pszFont, int fontPointSize, uint flags, IDirect3DDevice9 * pD3dDevice, IRenderFont * * ppFont)
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

   int height = -MulDiv(fontPointSize, GetDeviceCaps(hScreenDC, LOGPIXELSY), 72);

   ReleaseDC(NULL, hScreenDC), hScreenDC = NULL;

   bool bBold = (flags & kRFF_Bold) == kRFF_Bold;
   bool bItalic = (flags & kRFF_Italic) == kRFF_Italic;

   cAutoIPtr<ID3DXFont> pD3DXFont;
   if (D3DXCreateFont(pD3dDevice, height, 0, bBold ? FW_EXTRABOLD : FW_NORMAL,
      4, bItalic, DEFAULT_CHARSET, OUT_TT_PRECIS, PROOF_QUALITY,
      DEFAULT_PITCH | FF_DONTCARE, pszFont, &pD3DXFont) == S_OK)
   {
      cAutoIPtr<cRenderFontD3DX> pFont(new cRenderFontD3DX(pD3DXFont));
      if (!pFont)
      {
         return E_OUTOFMEMORY;
      }

      *ppFont = CTAddRef(pFont);
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
