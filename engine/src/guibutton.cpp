///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibutton.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "scriptapi.h"

#include "font.h"
#include "color.h"
#include "render.h"

#include "globalobj.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GUIButtonEvents);

#define LocalMsg(msg) DebugMsgEx(GUIButtonEvents, (msg))
#define LocalMsg1(msg,a1) DebugMsgEx1(GUIButtonEvents, (msg), (a1))
#define LocalMsg2(msg,a1,a2) DebugMsgEx2(GUIButtonEvents, (msg), (a1), (a2))

#define LocalMsgIf(cond,msg) DebugMsgIfEx(GUIButtonEvents, (cond), (msg))
#define LocalMsgIf1(cond,msg,a1) DebugMsgIfEx1(GUIButtonEvents, (cond), (msg), (a1))
#define LocalMsgIf2(cond,msg,a1,a2) DebugMsgIfEx2(GUIButtonEvents, (cond), (msg), (a1), (a2))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElement
//

///////////////////////////////////////

cGUIButtonElement::cGUIButtonElement()
 : m_bArmed(false),
   m_text("")
{
}

///////////////////////////////////////

cGUIButtonElement::~cGUIButtonElement()
{
}

///////////////////////////////////////

tResult cGUIButtonElement::OnEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tResult result = S_OK; // allow event processing to continue

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   LocalMsgIf(eventCode == kGUIEventMouseEnter, "Mouse enter button\n");
   LocalMsgIf(eventCode == kGUIEventMouseLeave, "Mouse leave button\n");

   if (eventCode == kGUIEventDragStart)
   {
      SetArmed(true);
      Verify(pEvent->SetCancelBubble(true) == S_OK);
      LocalMsg("Button drag start\n");
   }
   else if (eventCode == kGUIEventDragEnd)
   {
      SetArmed(false);
      Verify(pEvent->SetCancelBubble(true) == S_OK);
      LocalMsg("Button drag end\n");
   }
   else if (eventCode == kGUIEventDragMove)
   {
      Verify(pEvent->SetCancelBubble(true) == S_OK);
      LocalMsg("Button drag move\n");
   }
   else if (eventCode == kGUIEventClick)
   {
      SetArmed(false);
      LocalMsg("Button click\n");
      tGUIString onClick;
      if (GetOnClick(&onClick) == S_OK)
      {
         UseGlobal(ScriptInterpreter);
         pScriptInterpreter->ExecString(onClick.c_str());
         result = S_FALSE;
      }
   }

   return result;
}

///////////////////////////////////////

tResult cGUIButtonElement::GetRendererClass(tGUIString * pRendererClass)
{
   if (pRendererClass == NULL)
   {
      return E_POINTER;
   }
   *pRendererClass = "button";
   return S_OK;
}

///////////////////////////////////////

bool cGUIButtonElement::IsArmed() const
{
   return m_bArmed;
}

///////////////////////////////////////

void cGUIButtonElement::SetArmed(bool bArmed)
{
   m_bArmed = bArmed;
}

///////////////////////////////////////

const char * cGUIButtonElement::GetText() const
{
   return m_text;
}

///////////////////////////////////////

void cGUIButtonElement::SetText(const char * pszText)
{
   m_text = pszText;
}

///////////////////////////////////////

tResult cGUIButtonElement::GetOnClick(tGUIString * pOnClick) const
{
   if (pOnClick == NULL)
      return E_POINTER;
   if (m_onClick.empty())
      return S_FALSE;
   *pOnClick = m_onClick;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIButtonElement::SetOnClick(const char * pszOnClick)
{
   if (pszOnClick == NULL)
      return E_POINTER;
   m_onClick = pszOnClick;
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(button, cGUIButtonElementFactory);

tResult cGUIButtonElementFactory::CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), "button") == 0)
      {
         cAutoIPtr<IGUIButtonElement> pButton = static_cast<IGUIButtonElement *>(new cGUIButtonElement);
         if (!!pButton)
         {
            GUIElementStandardAttributes(pXmlElement, pButton);

            const char * pszAttribute;

            if ((pszAttribute = pXmlElement->Attribute("text")) != NULL)
            {
               pButton->SetText(pszAttribute);
            }

            if ((pszAttribute = pXmlElement->Attribute("onclick")) != NULL)
            {
               pButton->SetOnClick(pszAttribute);
            }

            *ppElement = CTAddRef(pButton);
            return S_OK;
         }
      }
   }
   else
   {
      *ppElement = static_cast<IGUIButtonElement *>(new cGUIButtonElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonRenderer
//

///////////////////////////////////////

cGUIButtonRenderer::cGUIButtonRenderer()
{
}

///////////////////////////////////////

cGUIButtonRenderer::~cGUIButtonRenderer()
{
}

///////////////////////////////////////

tResult cGUIButtonRenderer::Render(IGUIElement * pElement, IRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   static const float g_bevel = 2;

   cAutoIPtr<IGUIButtonElement> pButton;
   if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButton) == S_OK)
   {
      tGUIPoint pos = GUIElementAbsolutePosition(pButton);
      tGUISize size = pButton->GetSize();

      tVec2 textOffset(0,0);

      tRect rect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));
      tGUIRect rect2(rect.left, rect.top, rect.right, rect.bottom);

      UseGlobal(GUIRenderingTools);

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

      if (pButton->IsArmed() && pButton->IsMouseOver())
      {
         pGUIRenderingTools->Render3dRect(rect2, g_bevel, 
            tGUIColor::DarkGray, tGUIColor::LightGray, tGUIColor::Gray);
         textOffset = tVec2(g_bevel, g_bevel);
      }
      else
      {
         pGUIRenderingTools->Render3dRect(rect2, g_bevel, 
            tGUIColor::LightGray, tGUIColor::DarkGray, tGUIColor::Gray);
      }

      rect.left += Round(textOffset.x);
      rect.top += Round(textOffset.y);
      pFont->DrawText(pButton->GetText(), -1, kDT_Center | kDT_VCenter | 
         kDT_SingleLine, &rect, tGUIColor::White);

      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUIButtonRenderer::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      cAutoIPtr<IGUIButtonElement> pButton;
      if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButton) == S_OK)
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

         tRect rect(0,0,0,0);
         pFont->DrawText(pButton->GetText(), -1, kDT_CalcRect, &rect, tGUIColor::White);

         return tGUISize(rect.GetWidth() + rect.GetHeight(), rect.GetHeight() * 1.5f);
      }
   }
   return tGUISize(0,0);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(button, cGUIButtonRendererFactory);

tResult cGUIButtonRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   *ppRenderer = static_cast<IGUIElementRenderer *>(new cGUIButtonRenderer);
   return (*ppRenderer != NULL) ? S_OK : E_OUTOFMEMORY;
}

///////////////////////////////////////////////////////////////////////////////
