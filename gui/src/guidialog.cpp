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

static const int kDialog3dEdge = 2;

static const uint kNoCaptionHeight = (uint)-1;

///////////////////////////////////////

cGUIDialogElement::cGUIDialogElement()
 : m_bDragging(false),
   m_dragOffset(0,0),
   m_captionHeight(kNoCaptionHeight),
   m_bModal(false)
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

   tResult result = S_OK;

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   long keyCode;
   Verify(pEvent->GetKeyCode(&keyCode) == S_OK);

   tGUIPoint mousePos;
   Verify(pEvent->GetMousePosition(&mousePos) == S_OK);

   if (eventCode == kGUIEventMouseEnter)
   {
      LocalMsg("Mouse enter dialog\n");
   }
   else if (eventCode == kGUIEventMouseLeave)
   {
      LocalMsg("Mouse leave dialog\n");
   }
   else if (eventCode == kGUIEventMouseMove)
   {
      LocalMsg("Mouse move dialog\n");
      if (m_bDragging)
      {
         SetPosition(mousePos - m_dragOffset);
      }
   }
   else if (eventCode == kGUIEventMouseDown)
   {
      LocalMsg("Mouse down dialog\n");
      tGUIRect captionRect = GetCaptionRectAbsolute();
      if (captionRect.PtInside(Round(mousePos.x), Round(mousePos.y)))
      {
         UseGlobal(GUIContext);
         pGUIContext->SetCapture(this);
         m_bDragging = true;
         m_dragOffset = mousePos - tGUIPoint(captionRect.left, captionRect.top);
         result = S_FALSE; // now eat this event
      }
   }
   else if (eventCode == kGUIEventMouseUp)
   {
      LocalMsg("Mouse up dialog\n");
      UseGlobal(GUIContext);
      pGUIContext->SetCapture(NULL);
      m_bDragging = false;
      result = S_FALSE; // now eat this event
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

   return result;
}

///////////////////////////////////////

tResult cGUIDialogElement::GetRendererClass(tGUIString * pRendererClass)
{
   if (pRendererClass == NULL)
   {
      return E_POINTER;
   }
   *pRendererClass = "dialog";
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::GetInsets(tGUIInsets * pInsets)
{
   tResult result;
   if ((result = cGUIContainerBase<IGUIDialogElement>::GetInsets(pInsets)) == S_OK)
   {
      // Add in the caption height so that the layout manager can account for it
      // when sizing and positioning child elements
      uint captionHeight;
      if (GetCaptionHeight(&captionHeight) == S_OK)
      {
         Assert(pInsets != NULL); // if above call returned S_OK this should never be NULL
         pInsets->top += captionHeight;
      }
   }
   return result;
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

///////////////////////////////////////

tResult cGUIDialogElement::GetCaptionHeight(uint * pHeight)
{
   if (pHeight == NULL)
   {
      return E_POINTER;
   }

   if (m_captionHeight == kNoCaptionHeight)
   {
      return S_FALSE;
   }

   *pHeight = m_captionHeight;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::SetCaptionHeight(uint height)
{
   m_captionHeight = height;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::SetModal(bool bModal)
{
   m_bModal = bModal;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::IsModal()
{
   return m_bModal ? S_OK : S_FALSE;
}

///////////////////////////////////////

tGUIRect cGUIDialogElement::GetCaptionRectAbsolute()
{
   uint captionHeight;
   if ((GetCaptionHeight(&captionHeight) == S_OK) && (captionHeight > 0))
   {
      tGUIPoint pos = GUIElementAbsolutePosition(this);
      tGUISize size = GetSize();

      tGUIRect captionRect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));
      captionRect.left += kDialog3dEdge;
      captionRect.top += kDialog3dEdge;
      captionRect.right -= kDialog3dEdge;
      captionRect.bottom = captionRect.top + captionHeight;

      return captionRect;
   }

   return tGUIRect(0,0,0,0);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(dialog, cGUIDialogElementFactory);

static tResult QueryBoolAttribute(const TiXmlElement * pXmlElement, const char * pszAttrib, bool * pBool)
{
   Assert(pXmlElement != NULL);
   Assert(pszAttrib != NULL);
   Assert(pBool != NULL);

   int value;
	if (pXmlElement->QueryIntAttribute(pszAttrib, &value) == TIXML_SUCCESS)
   {
      *pBool = value ? true : false;
      return S_OK;
   }
   else
   {
      const char * pszValue = pXmlElement->Attribute(pszAttrib);
      if (pszValue != NULL)
      {
         if (stricmp(pszValue, "true") == 0)
         {
            *pBool = true;
            return S_OK;
         }
         else if (stricmp(pszValue, "false") == 0)
         {
            *pBool = false;
            return S_OK;
         }
      }
   }

   return S_FALSE;
}

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

            const char * pszValue;
            if ((pszValue = pXmlElement->Attribute("title")) != NULL)
            {
               pDialog->SetTitle(pszValue);
            }

            bool bIsModal;
            if (QueryBoolAttribute(pXmlElement, "ismodal", &bIsModal) == S_OK)
            {
               pDialog->SetModal(bIsModal);
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
      tGUIRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));

      UseGlobal(GUIRenderingTools);

      tGUIColor topLeft(tGUIColor::LightGray);
      tGUIColor bottomRight(tGUIColor::DarkGray);
      tGUIColor face(tGUIColor::Gray);
      tGUIColor caption(tGUIColor::Blue);

      cAutoIPtr<IRenderFont> pFont;

      cAutoIPtr<IGUIStyle> pStyle;
      if (pElement->GetStyle(&pStyle) == S_OK)
      {
         pStyle->GetFont(&pFont);
         pStyle->GetAttribute("frame-top-left-color", &topLeft);
         pStyle->GetAttribute("frame-bottom-right-color", &bottomRight);
         pStyle->GetAttribute("frame-face-color", &face);
         pStyle->GetAttribute("caption-color", &caption);
      }

      if (!pFont)
      {
         UseGlobal(GUIRenderingTools);
         pGUIRenderingTools->GetDefaultFont(&pFont);
      }

      pGUIRenderingTools->Render3dRect(rect, kDialog3dEdge, topLeft, bottomRight, face);

      uint captionHeight;
      if ((pDialog->GetCaptionHeight(&captionHeight) == S_OK)
         && (captionHeight > 0))
      {
         tGUIRect captionRect(rect);

         captionRect.left += kDialog3dEdge;
         captionRect.top += kDialog3dEdge;
         captionRect.right -= kDialog3dEdge;
         captionRect.bottom = captionRect.top + captionHeight;

         pGUIRenderingTools->Render3dRect(captionRect, 0, caption, caption, caption);

         tGUIString title;
         if (pDialog->GetTitle(&title) == S_OK)
         {
            pFont->DrawText(title.c_str(), -1, 0, &captionRect, tGUIColor::White);
         }
      }

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
         cAutoIPtr<IRenderFont> pFont;

         cAutoIPtr<IGUIStyle> pStyle;
         if (pElement->GetStyle(&pStyle) == S_OK)
         {
            pStyle->GetFont(&pFont);
         }

         if (!pFont)
         {
            UseGlobal(GUIRenderingTools);
            pGUIRenderingTools->GetDefaultFont(&pFont);
         }

         cAutoIPtr<IGUILayoutManager> pLayout;
         if (pDialog->GetLayout(&pLayout) == S_OK)
         {
            tGUISize size(0,0);
            if (pLayout->GetPreferredSize(pDialog, &size) == S_OK)
            {
               uint captionHeight;
               if (pDialog->GetCaptionHeight(&captionHeight) == S_OK)
               {
                  size.height += captionHeight;
               }
               else
               {
                  tGUIString title;
                  if (pDialog->GetTitle(&title) == S_OK)
                  {
                     tRect rect(0,0,0,0);
                     pFont->DrawText(title.c_str(), -1, kDT_CalcRect, &rect, tGUIColor::White);

                     captionHeight = rect.GetHeight();

                     pDialog->SetCaptionHeight(captionHeight);
                     size.height += captionHeight;
                  }
               }

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
