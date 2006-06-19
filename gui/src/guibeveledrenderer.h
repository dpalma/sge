///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBEVELEDRENDERER_H
#define INCLUDED_GUIBEVELEDRENDERER_H

#include "guiapi.h"
#include "guielementapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBeveledRenderer
//

enum eBeveledColors
{
   kBC_Shadow,
   kBC_Light,
   kBC_Face,
   kBC_Text,

   kBC_NumColors
};

class cGUIBeveledRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUIBeveledRenderer();
   ~cGUIBeveledRenderer();

   virtual tResult Render(IGUIElement * pElement, const tGUIPoint & position, IGUIRenderDevice * pRenderDevice);
   virtual tResult GetPreferredSize(IGUIElement * pElement, const tGUISize & parentSize, tGUISize * pSize);
   virtual tResult AllocateBorderSpace(IGUIElement * pElement, tGUIRect * pRect);

private:
   tResult ButtonRender(IGUIElement * pElement, const tGUIPoint & position, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize ButtonPreferredSize(IGUIElement * pElement, const tGUISize & parentSize) const;

   tResult LabelRender(IGUIElement * pElement, const tGUIPoint & position, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize LabelPreferredSize(IGUIElement * pElement, const tGUISize & parentSize) const;

   tResult ListBoxRender(IGUIElement * pElement, const tGUIPoint & position, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize ListBoxPreferredSize(IGUIElement * pElement, const tGUISize & parentSize) const;

   tResult ScrollBarRender(IGUIElement * pElement, const tGUIPoint & position, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize ScrollBarPreferredSize(IGUIElement * pElement, const tGUISize & parentSize) const;

   tResult TextEditRender(IGUIElement * pElement, const tGUIPoint & position, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize TextEditPreferredSize(IGUIElement * pElement, const tGUISize & parentSize) const;

   tResult TitleBarRender(IGUIElement * pElement, const tGUIPoint & position, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize TitleBarPreferredSize(IGUIElement * pElement, const tGUISize & parentSize) const;

   tResult ContainerRender(IGUIElement * pElement, const tGUIPoint & position, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize ContainerPreferredSize(IGUIElement * pElement, const tGUISize & parentSize) const;

   typedef tResult (cGUIBeveledRenderer::*tRenderMethod)(IGUIElement *, const tGUIPoint &, int, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice *);
   typedef tGUISize (cGUIBeveledRenderer::*tPreferredSizeMethod)(IGUIElement *, const tGUISize &) const;

   struct sMethodTableEntry
   {
      const IID * pIID;
      tRenderMethod pfnRender;
      tPreferredSizeMethod pfnPreferredSize;
   };

   static const sMethodTableEntry gm_methodTable[];
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIBEVELEDRENDERER_H
