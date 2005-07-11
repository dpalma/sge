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

class cGUIRenderDeviceGL : public cComObject<IMPLEMENTS(IGUIRenderDevice)>
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

private:
   long m_scissorRectStackDepth; // for debugging only
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_GUIRENDER_H
