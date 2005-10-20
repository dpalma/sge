///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBEVELEDRENDERER_H
#define INCLUDED_GUIBEVELEDRENDERER_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBeveledRenderer
//

class cGUIBeveledRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUIBeveledRenderer();
   ~cGUIBeveledRenderer();

   virtual tResult Render(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice);
   virtual tGUISize GetPreferredSize(IGUIElement * pElement);
   virtual tResult GetFont(IGUIElement * pElement, IGUIFont * * ppFont);

private:
   tResult Render(IGUIButtonElement * pButtonElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIDialogElement * pDialogElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUILabelElement * pLabelElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIPanelElement * pPanelElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUITextEditElement * pTextEditElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIScrollBarElement * pScrollBarElement, IGUIRenderDevice * pRenderDevice);

   tGUISize GetPreferredSize(IGUIButtonElement * pButtonElement);
   tGUISize GetPreferredSize(IGUIDialogElement * pDialogElement);
   tGUISize GetPreferredSize(IGUILabelElement * pLabelElement);
   tGUISize GetPreferredSize(IGUIPanelElement * pPanelElement);
   tGUISize GetPreferredSize(IGUITextEditElement * pTextEditElement);
   tGUISize GetPreferredSize(IGUIScrollBarElement * pScrollBarElement);
   tGUISize GetPreferredSize(IGUIContainerElement * pContainerElement);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBeveledRendererFactory
//

class cGUIBeveledRendererFactory : public cComObject<IMPLEMENTS(IGUIElementRendererFactory)>
{
public:
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer);

private:
   cAutoIPtr<IGUIElementRenderer> m_pStatelessBeveledRenderer;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIBEVELEDRENDERER_H
