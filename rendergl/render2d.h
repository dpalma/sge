///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDER2D_H
#define INCLUDED_RENDER2D_H

#include "render/renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRender2DGL
//

class cRender2DGL : public cComObject<IMPLEMENTS(IRender2D)>
{
   cRender2DGL(const cRender2DGL &);
   const cRender2DGL & operator =(const cRender2DGL &);

public:
   cRender2DGL();
   ~cRender2DGL();

   virtual tResult GetViewportSize(int * pWidth, int * pHeight) const;

   virtual void PushScissorRect(const tRect & rect);
   virtual void PopScissorRect();

   virtual void RenderSolidRect(const tRect & rect, const float color[4]);
   virtual void RenderBeveledRect(const tRect & rect, int bevel,
                                  const float topLeft[4],
                                  const float bottomRight[4],
                                  const float face[4]);

private:
   long m_scissorRectStackDepth; // for debugging only
   mutable int m_viewport[4];
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDER2D_H
