///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIRENDERGL_H
#define INCLUDED_GUIRENDERGL_H

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

   virtual void Begin2D(int width, int height);
   virtual void End2D();

private:
   long m_scissorRectStackDepth; // for debugging only
   int m_viewport[4];
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_GUIRENDERGL_H
