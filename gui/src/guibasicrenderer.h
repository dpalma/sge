///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBASICRENDERER_H
#define INCLUDED_GUIBASICRENDERER_H

#include "guirender.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBasicRenderer
//

class cGUIBasicRenderer : public cComObject<cGUIElementRenderer<cGUIBasicRenderer>,
                                            &IID_IGUIElementRenderer>
{
public:
   cGUIBasicRenderer();
   ~cGUIBasicRenderer();

   tResult Render(IGUIButtonElement * pButtonElement);
   tResult Render(IGUIDialogElement * pDialogElement);
   tResult Render(IGUILabelElement * pLabelElement);
   tResult Render(IGUIPanelElement * pPanelElement);
   tResult Render(IGUITextEditElement * pTextEditElement);

   tGUISize GetPreferredSize(IGUIButtonElement * pButtonElement);
   tGUISize GetPreferredSize(IGUIDialogElement * pDialogElement);
   tGUISize GetPreferredSize(IGUILabelElement * pLabelElement);
   tGUISize GetPreferredSize(IGUIPanelElement * pPanelElement);
   tGUISize GetPreferredSize(IGUITextEditElement * pTextEditElement);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBasicRendererFactory
//

class cGUIBasicRendererFactory : public cComObject<IMPLEMENTS(IGUIElementRendererFactory)>
{
public:
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer);

private:
   cAutoIPtr<IGUIElementRenderer> m_pStatelessBasicRenderer;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIBASICRENDERER_H
