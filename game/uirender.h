///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIRENDER_H
#define INCLUDED_UIRENDER_H

#include "uitypes.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(ITexture);
F_DECLARE_INTERFACE(IRenderFont);

///////////////////////////////////////////////////////////////////////////////

cUISize UIGetRootContainerSize();

void UIPushClipRect(const cUIRect & rect);
void UIPopClipRect();

enum eDrawTextFlags
{
   kTextDefault = 0,
   kTextCenter = (1<<0),
   kTextVCenter = (1<<1),
   kTextNoClip = (1<<2),
};

cUISize UIMeasureText(const char * pszText, int textLen, IRenderFont * pFont = NULL);

void UIDrawText(const char * pszText, int textLen,
                const cUIRect * pRect, uint flags = kTextDefault,
                IRenderFont * pFont = NULL,
                const cUIColor & color = cUIColor(1,1,1,1));

void UIDraw3dRect(const cUIRect & rect,
                  int bevel,
                  const cUIColor & topLeft,
                  const cUIColor & bottomRight,
                  const cUIColor & face);

void UIDrawGradientRect(const cUIRect & rect,
                        const cUIColor & topLeft,
                        const cUIColor & topRight,
                        const cUIColor & bottomRight,
                        const cUIColor & bottomLeft);

void UIDrawSolidRect(const cUIRect & rect, const cUIColor & color);

void UIDrawTextureRect(const cUIRect & rect, ITexture * pTexture);

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_UIRENDER_H
