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
F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IVertexDeclaration);
F_DECLARE_INTERFACE(IVertexBuffer);
F_DECLARE_INTERFACE(IIndexBuffer);

F_DECLARE_INTERFACE(IUIRenderingTools);

///////////////////////////////////////////////////////////////////////////////

cUISize UIGetRootContainerSize();

cUISize UIMeasureText(const char * pszText, int textLen, IRenderFont * pFont = NULL);

void UIDrawText(const char * pszText, int textLen,
                const cUIRect * pRect, uint flags = 0,
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
//
// INTERFACE: IUIRenderingTools
//

struct sUIVertex
{
   float u, v;
   byte r, g, b, a;
   tVec2 pos;
};

enum eUIButtonState
{
   kBS_Normal,
   kBS_Hover,
   kBS_Pressed,
   kBS_Disabled
};

const uint kNumBitmapButtonIndices = 4;
const uint kNumBitmapButtonVertices = 16; // 4 button states * 4 vertices per state

inline uint UIButtonStateVertexStartIndex(eUIButtonState buttonState)
{
   return buttonState * kNumBitmapButtonIndices;
}

interface IUIRenderingTools : IUnknown
{
   virtual tResult SetRenderDevice(IRenderDevice * pRenderDevice) = 0;
   virtual tResult GetRenderDevice(IRenderDevice * * ppRenderDevice) = 0;

   virtual tResult GetUIVertexDeclaration(IVertexDeclaration * * ppVertexDecl) = 0;

   // All bitmap buttons everywhere can share the same vertex and index
   // buffers by varying the material and start vertex used in the render call
   virtual tResult GetBitmapButtonIndexBuffer(IIndexBuffer * * ppIndexBuffer) = 0;
   virtual tResult GetBitmapButtonVertexBuffer(const cUIRect & rect, IVertexBuffer * * ppVertexBuffer) = 0;
};

void UIRenderingToolsCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_UIRENDER_H
