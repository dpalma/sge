///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guidialog.h"
#include "guielementbasetem.h"
#include "guicontainerbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"
#include "scriptapi.h"

#include "globalobj.h"
#include "keys.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GUIDialogEvents);

#define LocalMsg(msg) DebugMsgEx(GUIDialogEvents, (msg))
#define LocalMsg1(msg,a1) DebugMsgEx1(GUIDialogEvents, (msg), (a1))
#define LocalMsg2(msg,a1,a2) DebugMsgEx2(GUIDialogEvents, (msg), (a1), (a2))

#define LocalMsgIf(cond,msg) DebugMsgIfEx(GUIDialogEvents, (cond), (msg))
#define LocalMsgIf1(cond,msg,a1) DebugMsgIfEx1(GUIDialogEvents, (cond), (msg), (a1))
#define LocalMsgIf2(cond,msg,a1,a2) DebugMsgIfEx2(GUIDialogEvents, (cond), (msg), (a1), (a2))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogElement
//

static const uint kNoCaptionHeight = ~0u;

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

   LocalMsgIf(eventCode == kGUIEventClick, "Dialog clicked\n");
   LocalMsgIf(eventCode == kGUIEventMouseEnter, "Mouse enter dialog\n");
   LocalMsgIf(eventCode == kGUIEventMouseLeave, "Mouse leave dialog\n");

   switch (eventCode)
   {
      case kGUIEventDragStart:
      {
         LocalMsg("Dialog drag start\n");
         tGUIRect captionRect = GetCaptionRectAbsolute();
         if (captionRect.PtInside(Round(mousePos.x), Round(mousePos.y)))
         {
            m_bDragging = true;
            m_dragOffset = mousePos - GUIElementAbsolutePosition(this);
            pEvent->SetCancelBubble(true);
         }
         break;
      }

      case kGUIEventDragMove:
      {
         LocalMsg("Dialog drag move\n");
         if (m_bDragging)
         {
            SetPosition(mousePos - m_dragOffset);
         }
         pEvent->SetCancelBubble(true);
         // prevent the drag-over event since drag is being used to implement 
         // moving the dialog box
         result = S_FALSE;
         break;
      }

      case kGUIEventDragEnd:
      {
         LocalMsg("Dialog drag end\n");
         m_bDragging = false;
         pEvent->SetCancelBubble(true);
         break;
      }
   }

   if (eventCode == kGUIEventKeyUp)
   {
      LocalMsg("Key up dialog\n");
      if (keyCode == kEnter)
      {
         LocalMsg("ENTER KEY --> OK\n");
         pEvent->SetCancelBubble(true);
         Accept();
      }
   }
   else if (eventCode == kGUIEventKeyDown)
   {
      LocalMsg("Key down dialog\n");
      if (keyCode == kEscape)
      {
         LocalMsg("ESC KEY --> Cancel\n");
         pEvent->SetCancelBubble(true);
         Cancel();
      }
   }
   else if (eventCode == kGUIEventClick)
   {
      cAutoIPtr<IGUIElement> pSrcElement;
      if (pEvent->GetSourceElement(&pSrcElement) == S_OK)
      {
         cAutoIPtr<IGUIButtonElement> pButtonElement;
         if (pSrcElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButtonElement) == S_OK)
         {
            if (stricmp(pButtonElement->GetId(), "ok") == 0)
            {
               pEvent->SetCancelBubble(true);
               Accept();
            }
            else if (stricmp(pButtonElement->GetId(), "cancel") == 0)
            {
               pEvent->SetCancelBubble(true);
               Cancel();
            }
         }
      }
   }

   return result;
}

///////////////////////////////////////

tResult cGUIDialogElement::GetInsets(tGUIInsets * pInsets)
{
   tResult result;
   if ((result = tGUIDialogBase::GetInsets(pInsets)) == S_OK)
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

bool cGUIDialogElement::IsModal()
{
   return m_bModal;
}

///////////////////////////////////////

tResult cGUIDialogElement::Accept()
{
   tGUIString onOK;
   if (GetOnOK(&onOK) == S_OK)
   {
      UseGlobal(ScriptInterpreter);
      pScriptInterpreter->ExecString(onOK.c_str());
   }
   UseGlobal(GUIContext);
   pGUIContext->RemoveElement(this);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::Cancel()
{
   tGUIString onCancel;
   if (GetOnCancel(&onCancel) == S_OK)
   {
      UseGlobal(ScriptInterpreter);
      pScriptInterpreter->ExecString(onCancel.c_str());
   }
   UseGlobal(GUIContext);
   pGUIContext->RemoveElement(this);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::GetOnOK(tGUIString * pOnOK) const
{
   if (pOnOK == NULL)
      return E_POINTER;
   if (m_onOK.empty())
      return S_FALSE;
   *pOnOK = m_onOK;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::SetOnOK(const char * pszOnOK)
{
   if (pszOnOK == NULL)
      return E_POINTER;
   m_onOK = pszOnOK;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::GetOnCancel(tGUIString * pOnCancel) const
{
   if (pOnCancel == NULL)
      return E_POINTER;
   if (m_onCancel.empty())
      return S_FALSE;
   *pOnCancel = m_onCancel;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::SetOnCancel(const char * pszOnCancel)
{
   if (pszOnCancel == NULL)
      return E_POINTER;
   m_onCancel = pszOnCancel;
   return S_OK;
}

///////////////////////////////////////

const int kHackBevelValue = 2; // duplicates constant in beveled renderer

tGUIRect cGUIDialogElement::GetCaptionRectAbsolute()
{
   uint captionHeight;
   if ((GetCaptionHeight(&captionHeight) == S_OK) && (captionHeight > 0))
   {
      tGUIPoint pos = GUIElementAbsolutePosition(this);
      tGUISize size = GetSize();

      tGUIRect captionRect(Round(pos.x), Round(pos.y), Round(pos.x + size.width), Round(pos.y + size.height));
      captionRect.left += kHackBevelValue;
      captionRect.top += kHackBevelValue;
      captionRect.right -= kHackBevelValue;
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

extern tResult GUIStyleParseBool(const char * psz, bool * pBool);

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
         return GUIStyleParseBool(pszValue, pBool);
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
      if (strcmp(pXmlElement->Value(), kElementDialog) == 0)
      {
         cAutoIPtr<IGUIDialogElement> pDialog = static_cast<IGUIDialogElement *>(new cGUIDialogElement);
         if (!!pDialog)
         {
            GUIElementStandardAttributes(pXmlElement, pDialog);

            const char * pszValue;
            if ((pszValue = pXmlElement->Attribute(kAttribTitle)) != NULL)
            {
               pDialog->SetTitle(pszValue);
            }

            if ((pszValue = pXmlElement->Attribute(kAttribOnOk)) != NULL)
            {
               pDialog->SetOnOK(pszValue);
            }

            if ((pszValue = pXmlElement->Attribute(kAttribOnCancel)) != NULL)
            {
               pDialog->SetOnCancel(pszValue);
            }

            bool bIsModal;
            if (QueryBoolAttribute(pXmlElement, kAttribIsModal, &bIsModal) == S_OK)
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
