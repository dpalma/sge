///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guititlebar.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GUITitleBarEvents);

#define LocalMsg(msg)                  DebugMsgEx(GUITitleBarEvents, (msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(GUITitleBarEvents, (msg), (a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(GUITitleBarEvents, (msg), (a1), (a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUITitleBarEvents, (cond), (msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(GUITitleBarEvents, (cond), (msg), (a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(GUITitleBarEvents, (cond), (msg), (a1), (a2))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITitleBarElement
//

///////////////////////////////////////

cGUITitleBarElement::cGUITitleBarElement()
 : m_bDragging(false)
 , m_dragOffset(0,0)
{
}

///////////////////////////////////////

cGUITitleBarElement::~cGUITitleBarElement()
{
}

///////////////////////////////////////

tResult cGUITitleBarElement::OnEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tResult result = S_OK;

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   tScreenPoint mousePos;
   Verify(pEvent->GetMousePosition(&mousePos) == S_OK);

   switch (eventCode)
   {
      case kGUIEventDragStart:
      {
         tGUIRect rect = GetAbsoluteRect();
         if (rect.PtInside(mousePos.x, mousePos.y))
         {
            LocalMsg("TitleBar drag start\n");
            m_bDragging = true;
            // TODO: ADDED_tScreenPoint
            m_dragOffset.x = mousePos.x - rect.left;
            m_dragOffset.y = mousePos.y - rect.top;
            pEvent->SetCancelBubble(true);
         }
         break;
      }

      case kGUIEventDragMove:
      {
         if (m_bDragging)
         {
            LocalMsg("TitleBar drag move\n");
            cAutoIPtr<IGUIElement> pParent;
            if (GetParent(&pParent) == S_OK)
            {
               // TODO: ADDED_tScreenPoint
               tGUIPoint parentPos(
                  static_cast<float>(mousePos.x - m_dragOffset.x - GetPosition().x),
                  static_cast<float>(mousePos.y - m_dragOffset.y - GetPosition().y));
               pParent->SetPosition(parentPos);
            }
         }
         pEvent->SetCancelBubble(true);
         // Prevent the drag-over event since drag is being used to implement 
         // moving the parent element
         result = S_FALSE;
         break;
      }

      case kGUIEventDragEnd:
      {
         LocalMsg("TitleBar drag end\n");
         m_bDragging = false;
         pEvent->SetCancelBubble(true);
         break;
      }
   }

   return result;
}

///////////////////////////////////////

tResult cGUITitleBarElement::GetTitle(tGUIString * pTitle)
{
   if (pTitle == NULL)
   {
      return E_POINTER;
   }
   pTitle->assign(m_title);
   return m_title.empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cGUITitleBarElement::SetTitle(const tGUIChar * pszTitle)
{
   return GUISetText(pszTitle, &m_title);
}

///////////////////////////////////////

tGUIRect cGUITitleBarElement::GetAbsoluteRect()
{
   tGUIPoint pos = GUIElementAbsolutePosition(this);
   tGUISize size = GetSize();

   tGUIRect rect;
   rect.left = FloatToInt(pos.x);
   rect.top = FloatToInt(pos.y);
   rect.right = FloatToInt(pos.x + size.width);
   rect.bottom = FloatToInt(pos.y + size.height);

   return rect;
}

///////////////////////////////////////

tResult GUITitleBarCreate(IGUITitleBarElement * * ppTitleBarElement)
{
   if (ppTitleBarElement == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IGUITitleBarElement> pTitleBar(static_cast<IGUITitleBarElement*>(new cGUITitleBarElement));
   if (!pTitleBar)
   {
      return E_NOTIMPL;
   }
   return pTitleBar.GetPointer(ppTitleBarElement);
}


///////////////////////////////////////////////////////////////////////////////

tResult GUITitleBarElementCreate(const TiXmlElement * pXmlElement,
                                 IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementTitleBar) == 0)
      {
         cAutoIPtr<IGUITitleBarElement> pTitleBar(static_cast<IGUITitleBarElement *>(new cGUITitleBarElement));
         if (!pTitleBar)
         {
            return E_OUTOFMEMORY;
         }

         pTitleBar->SetTitle(pXmlElement->Attribute(kAttribTitle));

         *ppElement = CTAddRef(pTitleBar);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUITitleBarElement *>(new cGUITitleBarElement);
      return (*ppElement != NULL) ? S_OK : E_OUTOFMEMORY;
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(titlebar, GUITitleBarElementCreate);


///////////////////////////////////////////////////////////////////////////////
