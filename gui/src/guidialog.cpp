///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guidialog.h"
#include "guielementbasetem.h"
#include "guicontainerbasetem.h"
#include "guielementtools.h"

#include "font.h"
#include "color.h"
#include "render.h"

#include "globalobj.h"
#include "keys.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GUIDialogEvents);

#define LocalMsg(msg) DebugMsgEx(GUIDialogEvents, (msg))
#define LocalMsg1(msg,a1) DebugMsgEx1(GUIDialogEvents, (msg), (a1))
#define LocalMsg2(msg,a1,a2) DebugMsgEx2(GUIDialogEvents, (msg), (a1), (a2))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogElement
//

///////////////////////////////////////

cGUIDialogElement::cGUIDialogElement()
{
}

///////////////////////////////////////

cGUIDialogElement::~cGUIDialogElement()
{
}

///////////////////////////////////////

void cGUIDialogElement::SetSize(const tGUISize & size)
{
   tBaseClass::SetSize(size);

   cAutoIPtr<IGUILayoutManager> pLayout;
   if (GetLayout(&pLayout) == S_OK)
   {
      if (pLayout->Layout(this) != S_OK)
      {
         DebugMsg("ERROR: IGUILayoutManager::Layout() call failed for dialog box\n");
      }
   }
}

///////////////////////////////////////

tResult cGUIDialogElement::OnEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   long keyCode;
   Verify(pEvent->GetKeyCode(&keyCode) == S_OK);

   if (eventCode == kGUIEventMouseEnter)
   {
      LocalMsg("Mouse enter dialog\n");
   }
   else if (eventCode == kGUIEventMouseLeave)
   {
      LocalMsg("Mouse leave dialog\n");
   }
   else if (eventCode == kGUIEventMouseDown)
   {
      LocalMsg("Mouse down dialog\n");
   }
   else if (eventCode == kGUIEventMouseUp)
   {
      LocalMsg("Mouse up dialog\n");
   }
   else if (eventCode == kGUIEventClick)
   {
      LocalMsg("Mouse click dialog\n");
   }
   else if (eventCode == kGUIEventKeyUp)
   {
      LocalMsg("Key up dialog\n");
      if (keyCode == kEnter)
      {
         LocalMsg("ENTER KEY --> OK\n");
      }
   }
   else if (eventCode == kGUIEventKeyDown)
   {
      LocalMsg("Key down dialog\n");
      if (keyCode == kEscape)
      {
         LocalMsg("ESC KEY --> Cancel\n");
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::GetRendererClass(tGUIString * pRendererClass)
{
   if (pRendererClass == NULL)
      return E_POINTER;
   *pRendererClass = "dialog";
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::GetTitle(tGUIString * pTitle)
{
   if (pTitle == NULL)
   {
      return E_POINTER;
   }
   *pTitle = m_title;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::SetTitle(const char * pszTitle)
{
   if (pszTitle == NULL)
   {
      return E_POINTER;
   }
   m_title = pszTitle;
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(dialog, cGUIDialogElementFactory);

tResult cGUIDialogElementFactory::CreateElement(const TiXmlElement * pXmlElement, 
                                                IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), "dialog") == 0)
      {
         cAutoIPtr<IGUIDialogElement> pDialog = static_cast<IGUIDialogElement *>(new cGUIDialogElement);
         if (!!pDialog)
         {
            GUIElementStandardAttributes(pXmlElement, pDialog);

            if (pXmlElement->Attribute("title"))
            {
               pDialog->SetTitle(pXmlElement->Attribute("title"));
            }

            if (GUIElementCreateChildren(pXmlElement, pDialog) == S_OK)
            {
               *ppElement = CTAddRef(pDialog);
               return S_OK;
            }
         }
      }
   }
   else
   {
      *ppElement = static_cast<IGUIDialogElement *>(new cGUIDialogElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogRenderer
//

static const int k3dEdge = 2;

///////////////////////////////////////

cGUIDialogRenderer::cGUIDialogRenderer()
{
}

///////////////////////////////////////

cGUIDialogRenderer::~cGUIDialogRenderer()
{
}

///////////////////////////////////////

tResult cGUIDialogRenderer::Render(IGUIElement * pElement, IRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIDialogElement> pDialog;
   if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialog) == S_OK)
   {
      tGUIPoint pos = GUIElementAbsolutePosition(pDialog);
      tGUISize size = pDialog->GetSize();

      UseGlobal(GUIRenderingTools);

      // TODO: use colors from style
      pGUIRenderingTools->Render3dRect(
         tGUIRect(pos.x, pos.y, pos.x + size.width, pos.y + size.height), 
         k3dEdge, tGUIColor::LightGray, tGUIColor::DarkGray, tGUIColor::Gray);

      if (GUIElementRenderChildren(pDialog, pRenderDevice) == S_OK)
      {
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUIDialogRenderer::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      cAutoIPtr<IGUIDialogElement> pDialog;
      if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialog) == S_OK)
      {
         cAutoIPtr<IGUILayoutManager> pLayout;
         if (pDialog->GetLayout(&pLayout) == S_OK)
         {
            tGUISize size(0,0);
            if (pLayout->GetPreferredSize(pDialog, &size) == S_OK)
            {
               return size;
            }
         }
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(dialog, cGUIDialogRendererFactory);

tResult cGUIDialogRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, 
                                                  IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   *ppRenderer = static_cast<IGUIElementRenderer *>(new cGUIDialogRenderer);
   return (*ppRenderer != NULL) ? S_OK : E_OUTOFMEMORY;
}

///////////////////////////////////////////////////////////////////////////////
