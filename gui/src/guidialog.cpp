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

#define LocalMsg(msg)                  DebugMsgEx(GUIDialogEvents, (msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(GUIDialogEvents, (msg), (a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(GUIDialogEvents, (msg), (a1), (a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUIDialogEvents, (cond), (msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(GUIDialogEvents, (cond), (msg), (a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(GUIDialogEvents, (cond), (msg), (a1), (a2))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogElement
//

static const uint kNoCaptionHeight = ~0u;

///////////////////////////////////////

cGUIDialogElement::cGUIDialogElement()
 : m_bDragging(false),
   m_dragOffset(0,0),
   m_captionHeight(kNoCaptionHeight)
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
   return m_title.empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cGUIDialogElement::SetTitle(const char * pszTitle)
{
   if (pszTitle == NULL)
   {
      m_title.erase();
   }
   else
   {
      m_title = pszTitle;
   }
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

tResult GUIDialogElementCreate(const TiXmlElement * pXmlElement,
                               IGUIElement * pParent, IGUIElement * * ppElement)
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
         if (!pDialog)
         {
            return E_OUTOFMEMORY;
         }

         GUIElementStandardAttributes(pXmlElement, pDialog);

         const char * pszValue = NULL;
         if ((pszValue = pXmlElement->Attribute(kAttribTitle)) != NULL)
         {
            pDialog->SetTitle(pszValue);
         }

         *ppElement = CTAddRef(pDialog);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUIDialogElement *>(new cGUIDialogElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(dialog, GUIDialogElementCreate);


///////////////////////////////////////////////////////////////////////////////
