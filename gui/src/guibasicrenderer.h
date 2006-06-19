///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBASICRENDERER_H
#define INCLUDED_GUIBASICRENDERER_H

#include "guiapi.h"
#include "guielementapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBasicRenderer
//

class cGUIBasicRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUIBasicRenderer();
   ~cGUIBasicRenderer();

   virtual tResult Render(IGUIElement * pElement, const tGUIPoint & position, IGUIRenderDevice * pRenderDevice);
   virtual tResult GetPreferredSize(IGUIElement * pElement, const tGUISize & parentSize, tGUISize * pSize);
   virtual tResult AllocateBorderSpace(IGUIElement * pElement, tGUIRect * pRect);

private:
   tResult LabelRender(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice);
   tGUISize LabelPreferredSize(IGUIElement * pElement) const;

   tResult PanelRender(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice);
   tGUISize PanelPreferredSize(IGUIElement * pElement) const;

   typedef tResult (cGUIBasicRenderer::*tRenderMethod)(IGUIElement *, IGUIRenderDevice *);
   typedef tGUISize (cGUIBasicRenderer::*tPreferredSizeMethod)(IGUIElement *) const;

   struct sMethodTableEntry
   {
      const IID * pIID;
      tRenderMethod pfnRender;
      tPreferredSizeMethod pfnPreferredSize;
   };

   static const sMethodTableEntry gm_methodTable[];
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIBASICRENDERER_H
