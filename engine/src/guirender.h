///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIRENDER_H
#define INCLUDED_GUIRENDER_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIRenderDeviceGL
//

class cGUIRenderDeviceGL : public cComObject<IMPLEMENTS(IGUIRenderDeviceContext)>
{
   cGUIRenderDeviceGL(const cGUIRenderDeviceGL &);
   void operator =(const cGUIRenderDeviceGL &);

public:
   cGUIRenderDeviceGL();
   ~cGUIRenderDeviceGL();

   virtual void PushScissorRect(const tGUIRect & rect);
   virtual void PopScissorRect();

   virtual void RenderSolidRect(const tGUIRect & rect, const tGUIColor & color);
   virtual void RenderBeveledRect(const tGUIRect & rect, int bevel, const tGUIColor & topLeft,
                                  const tGUIColor & bottomRight, const tGUIColor & face);

   virtual void FlushQueue();

   virtual void Begin2D();
   virtual void End2D();

   virtual tResult GetViewportSize(uint * pWidth, uint * pHeight);

private:
   long m_scissorRectStackDepth; // for debugging only
   int m_viewport[4];
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_GUIRENDER_H
