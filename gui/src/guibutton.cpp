///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibutton.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "script/scriptapi.h"
#include "guistrings.h"

#include "tech/globalobj.h"

#include <tinyxml.h>

#include "tech/dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GUIButtonEvents);

#define LocalMsg(msg)                  DebugMsgEx(GUIButtonEvents, (msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(GUIButtonEvents, (msg), (a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(GUIButtonEvents, (msg), (a1), (a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUIButtonEvents, (cond), (msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(GUIButtonEvents, (cond), (msg), (a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(GUIButtonEvents, (cond), (msg), (a1), (a2))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElement
//

///////////////////////////////////////

cGUIButtonElement::cGUIButtonElement()
 : m_bArmed(false)
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
      // prevent the drag-over event since drag is being used to implement 
      // the "arming" of the button
      result = S_FALSE;
      LocalMsg("Button drag move\n");
   }
   else if (eventCode == kGUIEventClick)
   {
      SetArmed(false);
      LocalMsg("Button click\n");
      tGUIString onClick;
      if (GetOnClick(&onClick) == S_OK)
      {
         Verify(pEvent->SetCancelBubble(true) == S_OK);
         UseGlobal(ScriptInterpreter);
         pScriptInterpreter->ExecString(onClick.c_str());
      }
   }

   return result;
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

const tGUIChar * cGUIButtonElement::GetText() const
{
   return m_text.c_str();
}

///////////////////////////////////////

tResult cGUIButtonElement::GetText(tGUIString * pText) const
{
   if (pText == NULL)
   {
      return E_POINTER;
   }
   pText->assign(m_text);
   return m_text.empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cGUIButtonElement::SetText(const tGUIChar * pszText)
{
   return GUISetText(pszText, &m_text);
}

///////////////////////////////////////

tResult cGUIButtonElement::GetOnClick(tGUIString * pOnClick) const
{
   if (pOnClick == NULL)
   {
      return E_POINTER;
   }
   if (m_onClick.empty())
   {
      return S_FALSE;
   }
   *pOnClick = m_onClick;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIButtonElement::SetOnClick(const char * pszOnClick)
{
   if (pszOnClick == NULL)
   {
      m_onClick.erase();
   }
   else
   {
      m_onClick.assign(pszOnClick);
   }
   return S_OK;
}

///////////////////////////////////////

tResult GUIButtonCreate(IGUIButtonElement * * ppButtonElement)
{
   cAutoIPtr<IGUIButtonElement> pButton(static_cast<IGUIButtonElement*>(new cGUIButtonElement));
   if (!pButton)
   {
      return E_NOTIMPL;
   }
   return pButton.GetPointer(ppButtonElement);
}


///////////////////////////////////////////////////////////////////////////////

tResult GUIButtonElementCreate(const TiXmlElement * pXmlElement,
                               IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementButton) == 0)
      {
         cAutoIPtr<IGUIButtonElement> pButton = static_cast<IGUIButtonElement *>(new cGUIButtonElement);
         if (!pButton)
         {
            return E_OUTOFMEMORY;
         }

         pButton->SetText(pXmlElement->Attribute(kAttribText));
         pButton->SetOnClick(pXmlElement->Attribute(kAttribOnClick));

         *ppElement = CTAddRef(pButton);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUIButtonElement *>(new cGUIButtonElement);
      return (*ppElement != NULL) ? S_OK : E_OUTOFMEMORY;
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(button, GUIButtonElementCreate);

///////////////////////////////////////////////////////////////////////////////
