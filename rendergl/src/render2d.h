///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDER2D_H
#define INCLUDED_RENDER2D_H

#include "renderapi.h"

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
   void operator =(const cRender2DGL &);

public:
   cRender2DGL();
   ~cRender2DGL();

   virtual tResult GetViewportSize(int * pWidth, int * pHeight) const;

   virtual void PushScissorRect(const tRect & rect);
   virtual void PopScissorRect();

   virtual void RenderSolidRect(const tRect & rect, const cColor & color);
   virtual void RenderBeveledRect(const tRect & rect, int bevel, const cColor & topLeft,
                                  const cColor & bottomRight, const cColor & face);

private:
   long m_scissorRectStackDepth; // for debugging only
   mutable int m_viewport[4];
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDER2D_H
