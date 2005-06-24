///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibasicrenderer.h"
#include "guielementtools.h"
#include "guistrings.h"
#include "scriptapi.h"

#include "color.h"

#include "globalobj.h"

#include <tinyxml.h>
#include <GL/glew.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBasicRenderer
//

static const int g_bevel = 2;

///////////////////////////////////////

cGUIBasicRenderer::cGUIBasicRenderer()
{
}

///////////////////////////////////////

cGUIBasicRenderer::~cGUIBasicRenderer()
{
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUIButtonElement * pButtonElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUIDialogElement * pDialogElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUILabelElement * pLabelElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUIPanelElement * pPanelElement)
{
   tGUIPoint pos = GUIElementAbsolutePosition(pPanelElement);
   tGUISize size = pPanelElement->GetSize();
   tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

   cAutoIPtr<IGUIStyle> pStyle;
   if (pPanelElement->GetStyle(&pStyle) == S_OK)
   {
      tGUIColor bkColor;
      if (pStyle->GetBackgroundColor(&bkColor) == S_OK)
      {
         GlRenderBevelledRect(rect, 0, bkColor, bkColor, bkColor);
      }
   }

   return GUIElementRenderChildren(pPanelElement);
}

///////////////////////////////////////

tResult cGUIBasicRenderer::Render(IGUITextEditElement * pTextEditElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUIButtonElement * pButtonElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUIDialogElement * pDialogElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUILabelElement * pLabelElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUIPanelElement * pPanelElement)
{
   return cGUIElementRenderer<cGUIBasicRenderer>::GetPreferredSize(static_cast<IGUIContainerElement*>(pPanelElement));
}

///////////////////////////////////////

tGUISize cGUIBasicRenderer::GetPreferredSize(IGUITextEditElement * pTextEditElement)
{
   return tGUISize(0,0);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBasicRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(basic, cGUIBasicRendererFactory);

////////////////////////////////////////

tResult cGUIBasicRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   // Since the renderer is stateless, a single instance can serve all GUI elements
   if (!m_pStatelessBasicRenderer)
   {
      m_pStatelessBasicRenderer = static_cast<IGUIElementRenderer *>(new cGUIBasicRenderer);
      if (!m_pStatelessBasicRenderer)
      {
         return E_OUTOFMEMORY;
      }
   }

   *ppRenderer = CTAddRef(m_pStatelessBasicRenderer);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
