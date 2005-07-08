///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBASICRENDERER_H
#define INCLUDED_GUIBASICRENDERER_H

#include "guiapi.h"

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

inline IGUIFont * cGUIBasicRenderer::AccessDefaultFont()
{
   if (!m_pDefaultFont)
   {
      GUIFontGetDefault(&m_pDefaultFont);
   }
   return m_pDefaultFont;
}

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
