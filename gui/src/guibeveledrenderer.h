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

   virtual tResult Render(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice);
   tGUISize GetPreferredSize(IGUIElement * pElement);
   virtual tResult GetPreferredSize(IGUIElement * pElement, tGUISize * pSize);

private:
   tResult ButtonRender(IGUIElement * pElement, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize ButtonPreferredSize(IGUIElement * pElement) const;

   tResult ListBoxRender(IGUIElement * pElement, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize ListBoxPreferredSize(IGUIElement * pElement) const;

   tResult TitleBarRender(IGUIElement * pElement, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tGUISize TitleBarPreferredSize(IGUIElement * pElement) const;

   tGUISize ContainerPreferredSize(IGUIElement * pElement) const;

   tResult Render(IGUIDialogElement * pDialogElement, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUILabelElement * pLabelElement, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIPanelElement * pPanelElement, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUITextEditElement * pTextEditElement, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIScrollBarElement * pScrollBarElement, int bevel, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice * pRenderDevice);

   tGUISize GetPreferredSize(IGUIDialogElement * pDialogElement);
   tGUISize GetPreferredSize(IGUILabelElement * pLabelElement);
   tGUISize GetPreferredSize(IGUIPanelElement * pPanelElement);
   tGUISize GetPreferredSize(IGUITextEditElement * pTextEditElement);
   tGUISize GetPreferredSize(IGUIScrollBarElement * pScrollBarElement);

   typedef tResult (cGUIBeveledRenderer::*tRenderMethod)(IGUIElement *, int, const tGUIColor colors[kBC_NumColors], IGUIRenderDevice *);
   typedef tGUISize (cGUIBeveledRenderer::*tPreferredSizeMethod)(IGUIElement *) const;

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
