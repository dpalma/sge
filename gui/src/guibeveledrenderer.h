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
   tGUISize GetPreferredSize(IGUIElement * pElement);
   virtual tResult GetPreferredSize(IGUIElement * pElement, tGUISize * pSize);
   virtual tResult ComputeClientArea(IGUIElement * pElement, tGUIRect * pClientArea);
   virtual tResult GetFont(IGUIElement * pElement, IGUIFont * * ppFont);

private:
   tResult Render(IGUIButtonElement * pButtonElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIDialogElement * pDialogElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUILabelElement * pLabelElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIPanelElement * pPanelElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUITextEditElement * pTextEditElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIScrollBarElement * pScrollBarElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIListBoxElement * pListBoxElement, IGUIRenderDevice * pRenderDevice);

   tGUISize GetPreferredSize(IGUIButtonElement * pButtonElement);
   tGUISize GetPreferredSize(IGUIDialogElement * pDialogElement);
   tGUISize GetPreferredSize(IGUILabelElement * pLabelElement);
   tGUISize GetPreferredSize(IGUIPanelElement * pPanelElement);
   tGUISize GetPreferredSize(IGUITextEditElement * pTextEditElement);
   tGUISize GetPreferredSize(IGUIScrollBarElement * pScrollBarElement);
   tGUISize GetPreferredSize(IGUIListBoxElement * pListBoxElement);
   tGUISize GetPreferredSize(IGUIContainerElement * pContainerElement);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIBEVELEDRENDERER_H
