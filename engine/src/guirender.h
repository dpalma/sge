///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIRENDER_H
#define INCLUDED_GUIRENDER_H

#include "comtools.h"
#include "guitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);
F_DECLARE_INTERFACE(IRenderDevice);
F_DECLARE_INTERFACE(IVertexDeclaration);
F_DECLARE_INTERFACE(IVertexBuffer);
F_DECLARE_INTERFACE(IIndexBuffer);

F_DECLARE_INTERFACE(IGUIRenderingTools);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIRenderingTools
//

interface IGUIRenderingTools : IUnknown
{
   virtual tResult SetRenderDevice(IRenderDevice * pRenderDevice) = 0;
   virtual tResult GetRenderDevice(IRenderDevice * * ppRenderDevice) = 0;

   virtual tResult SetDefaultFont(IRenderFont * pFont) = 0;
   virtual tResult GetDefaultFont(IRenderFont * * ppFont) = 0;

   // All bitmap buttons everywhere can share the same vertex and index
   // buffers by varying the material and start vertex used in the render call
   virtual tResult GetBitmapButtonIndexBuffer(IIndexBuffer * * ppIndexBuffer) = 0;
   virtual tResult GetBitmapButtonVertexBuffer(const tGUIRect & rect, IVertexBuffer * * ppVertexBuffer) = 0;

   virtual tResult Render3dRect(const tGUIRect & rect, int bevel, 
      const tGUIColor & topLeft, const tGUIColor & bottomRight, const tGUIColor & face) = 0;
};

///////////////////////////////////////

void GUIRenderingToolsCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_GUIRENDER_H
