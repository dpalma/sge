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

   virtual tResult Render(IGUIElement * pElement);
   virtual tGUISize GetPreferredSize(IGUIElement * pElement);
   virtual tResult GetFont(IGUIElement * pElement, IGUIFont * * ppFont);

private:
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
   tGUISize GetPreferredSize(IGUIContainerElement * pContainerElement);

   IGUIFont * AccessDefaultFont();

   cAutoIPtr<IGUIFont> m_pDefaultFont;
};

///////////////////////////////////////

inline IGUIFont * cGUIBeveledRenderer::AccessDefaultFont()
{
   if (!m_pDefaultFont)
   {
      GUIFontGetDefault(&m_pDefaultFont);
   }
   return m_pDefaultFont;
}

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
