///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIRENDER_H
#define INCLUDED_GUIRENDER_H

#include "guiapi.h"

#include "renderapi.h"

#include "globalobj.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIRenderingTools
//

class cGUIRenderingTools : public cGlobalObject<IMPLEMENTS(IGUIRenderingTools)>
{
public:
   cGUIRenderingTools();
   ~cGUIRenderingTools();

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult SetRenderDevice(IRenderDevice * pRenderDevice);
   virtual tResult GetRenderDevice(IRenderDevice * * ppRenderDevice);

   virtual tResult SetDefaultFont(IRenderFont * pFont);
   virtual tResult GetDefaultFont(IRenderFont * * ppFont);

   virtual tResult GetBitmapButtonIndexBuffer(IIndexBuffer * * ppIndexBuffer);
   virtual tResult GetBitmapButtonVertexBuffer(const tGUIRect & rect, IVertexBuffer * * ppVertexBuffer);

   virtual tResult Render3dRect(const tGUIRect & rect, int bevel, 
      const tGUIColor & topLeft, const tGUIColor & bottomRight, const tGUIColor & face);

private:
   inline IRenderDevice * AccessRenderDevice() { return m_pRenderDevice; }

   tResult GetVertexDeclaration(IVertexDeclaration * * ppVertexDecl);
   tResult GetTexVertexDeclaration(IVertexDeclaration * * ppVertexDecl);

   struct sGUIVertex
   {
      uint32 color;
      tVec2 pos;
   };

   struct sGUITexVertex
   {
      float u,v;
      uint32 color;
      tVec2 pos;
   };

   static sVertexElement gm_vertexDecl[];
   static sVertexElement gm_texVertexDecl[];

   cAutoIPtr<IRenderDevice> m_pRenderDevice;
   cAutoIPtr<IVertexDeclaration> m_pVertexDecl;
   cAutoIPtr<IVertexDeclaration> m_pTexVertexDecl;
   cAutoIPtr<IVertexBuffer> m_pBitmapButtonVB;
   cAutoIPtr<IIndexBuffer> m_pBitmapButtonIB;
   cAutoIPtr<IRenderFont> m_pFont;
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_GUIRENDER_H
