///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guiscrollbar.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "tech/globalobj.h"

#include <tinyxml.h>

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(GUIScrollBar);

#define LocalMsg(msg)                  DebugMsgEx(GUIScrollBar,(msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(GUIScrollBar,(msg),(a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(GUIScrollBar,(msg),(a1),(a2))
#define LocalMsg3(msg,a1,a2,a3)        DebugMsgEx3(GUIScrollBar,(msg),(a1),(a2),(a3))
#define LocalMsg4(msg,a1,a2,a3,a4)     DebugMsgEx4(GUIScrollBar,(msg),(a1),(a2),(a3),(a4))
#define LocalMsg5(msg,a1,a2,a3,a4,a5)  DebugMsgEx5(GUIScrollBar,(msg),(a1),(a2),(a3),(a4),(a5))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUIScrollBar,(cond),(msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(GUIScrollBar,(cond),(msg),(a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(GUIScrollBar,(cond),(msg),(a1),(a2))
#define LocalMsgIf3(cond,msg,a1,a2,a3) DebugMsgIfEx3(GUIScrollBar,(cond),(msg),(a1),(a2),(a3))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIScrollBarElement
//

///////////////////////////////////////

tResult GUIScrollBarElementCreate(eGUIScrollBarType scrollBarType, IGUIScrollBarElement * * ppScrollBar)
{
   cAutoIPtr<IGUIScrollBarElement> pScrollBar(static_cast<IGUIScrollBarElement*>(new cGUIScrollBarElement(scrollBarType)));
   if (!pScrollBar)
   {
      return E_OUTOFMEMORY;
   }
   return pScrollBar.GetPointer(ppScrollBar);
}

///////////////////////////////////////

cGUIScrollBarElement::cGUIScrollBarElement(eGUIScrollBarType scrollBarType)
 : m_scrollBarType(scrollBarType)
 , m_armedPart(kGUIScrollBarPartNone)
 , m_mouseOverPart(kGUIScrollBarPartNone)
 , m_rangeMin(0)
 , m_rangeMax(100)
 , m_scrollPos(0)
 , m_lineSize(1)
 , m_pageSize(5)
{
}

///////////////////////////////////////

cGUIScrollBarElement::~cGUIScrollBarElement()
{
}

///////////////////////////////////////

tResult cGUIScrollBarElement::OnEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tResult result = S_OK; // allow event processing to continue

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   tScreenPoint point;
   Verify(pEvent->GetMousePosition(&point) == S_OK);

   if (eventCode == kGUIEventDragStart)
   {
      Verify(pEvent->SetCancelBubble(true) == S_OK);

      m_armedPart = m_mouseOverPart;
      m_dragOffset = DetermineScrollPos(point) - m_scrollPos;
      m_dragStartScrollPos = m_scrollPos;
   }
   else if (eventCode == kGUIEventDragEnd)
   {
      Verify(pEvent->SetCancelBubble(true) == S_OK);

      if (m_armedPart != m_mouseOverPart)
      {
         m_armedPart = kGUIScrollBarPartNone;
      }
   }
   else if (eventCode == kGUIEventDragMove)
   {
      Verify(pEvent->SetCancelBubble(true) == S_OK);

      // Prevent the drag-over event since drag is being used to implement 
      // the arming of the scroll buttons
      result = S_FALSE;

      if (m_armedPart == kGUIScrollBarPartThumb)
      {
         int newScrollPos = DetermineScrollPos(point);
         if (newScrollPos < 0)
         {
            SetScrollPos(m_dragStartScrollPos);
         }
         else
         {
            SetScrollPos(newScrollPos - m_dragOffset);
         }
      }
   }
   else if (eventCode == kGUIEventMouseMove)
   {
      m_mouseOverPart = GetHitPart(point);
   }
   else if (eventCode == kGUIEventMouseLeave)
   {
      m_mouseOverPart = kGUIScrollBarPartNone;
   }
   else if (eventCode == kGUIEventClick)
   {
      if (m_armedPart != kGUIScrollBarPartNone && m_armedPart == m_mouseOverPart)
      {
         switch (m_armedPart)
         {
            case kGUIScrollBarPartButton1:
            {
               SetScrollPos(m_scrollPos - m_lineSize);
               break;
            }
            case kGUIScrollBarPartButton2:
            {
               SetScrollPos(m_scrollPos + m_lineSize);
               break;
            }
            case kGUIScrollBarPartTrack1:
            {
               SetScrollPos(m_scrollPos - m_pageSize);
               break;
            }
            case kGUIScrollBarPartTrack2:
            {
               SetScrollPos(m_scrollPos + m_pageSize);
               break;
            }
         }
         result = S_FALSE;
      }
      m_armedPart = kGUIScrollBarPartNone;
   }

   return result;
}

///////////////////////////////////////

eGUIScrollBarType cGUIScrollBarElement::GetScrollBarType()
{
   return m_scrollBarType;
}

///////////////////////////////////////

eGUIScrollBarPart cGUIScrollBarElement::GetArmedPart()
{
   return m_armedPart;
}

///////////////////////////////////////

eGUIScrollBarPart cGUIScrollBarElement::GetMouseOverPart()
{
   return m_mouseOverPart;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::GetPartRect(eGUIScrollBarPart part, tGUIRect * pRect)
{
   if (pRect == NULL)
   {
      return E_POINTER;
   }

   tGUISize size(GetSize());
   int width = FloatToInt(size.width);
   int height = FloatToInt(size.height);

   if (m_scrollBarType == kGUIScrollBarHorizontal)
   {
      if (part == kGUIScrollBarPartButton1)
      {
         *pRect = tGUIRect(0, 0, height, height);
         return S_OK;
      }
      else if (part == kGUIScrollBarPartButton2)
      {
         *pRect = tGUIRect(width - height, 0, width, height);
         return S_OK;
      }
      else if (part == kGUIScrollBarPartThumb)
      {
         uint leftEdge = (((width - (3 * height)) * m_scrollPos) / (m_rangeMax - m_rangeMin)) + height;
         *pRect = tGUIRect(leftEdge, 0, leftEdge + height, height);
         return S_OK;
      }
   }
   else if (m_scrollBarType == kGUIScrollBarVertical)
   {
      if (part == kGUIScrollBarPartButton1)
      {
         *pRect = tGUIRect(0, 0, width, width);
         return S_OK;
      }
      else if (part == kGUIScrollBarPartButton2)
      {
         *pRect = tGUIRect(0, height - width, width, height);
         return S_OK;
      }
      else if (part == kGUIScrollBarPartThumb)
      {
         uint topEdge = (((height - (3 * width)) * m_scrollPos) / (m_rangeMax - m_rangeMin)) + width;
         *pRect = tGUIRect(0, topEdge, width, topEdge + width);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::GetRange(int * pRangeMin, int * pRangeMax)
{
   if (pRangeMin == NULL || pRangeMax == NULL)
   {
      return E_POINTER;
   }
   *pRangeMin = m_rangeMin;
   *pRangeMax = m_rangeMax;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::SetRange(int rangeMin, int rangeMax)
{
   if (rangeMin >= rangeMax)
   {
      return E_INVALIDARG;
   }
   m_rangeMin = rangeMin;
   m_rangeMax = rangeMax;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::GetScrollPos(int * pScrollPos)
{
   if (pScrollPos == NULL)
   {
      return E_POINTER;
   }
   *pScrollPos = m_scrollPos;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::SetScrollPos(int scrollPos)
{
   if (scrollPos < m_rangeMin)
   {
      scrollPos = m_rangeMin;
   }

   if (scrollPos > m_rangeMax)
   {
      scrollPos = m_rangeMax;
   }

   if (m_scrollPos != scrollPos)
   {
      m_scrollPos = scrollPos;
      LocalMsg1("New scroll position %d\n", m_scrollPos);
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cGUIScrollBarElement::GetLineSize(int * pLineSize)
{
   if (pLineSize == NULL)
   {
      return E_POINTER;
   }
   *pLineSize = m_lineSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::SetLineSize(int lineSize)
{
   if (lineSize > (m_rangeMin - m_rangeMax))
   {
      return E_INVALIDARG;
   }
   m_lineSize = lineSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::GetPageSize(int * pPageSize)
{
   if (pPageSize == NULL)
   {
      return E_POINTER;
   }
   *pPageSize = m_pageSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::SetPageSize(int pageSize)
{
   if (pageSize > (m_rangeMin - m_rangeMax))
   {
      return E_INVALIDARG;
   }
   m_pageSize = pageSize;
   return S_OK;
}

///////////////////////////////////////

int cGUIScrollBarElement::DetermineScrollPos(const tScreenPoint & mousePos) const
{
   tGUISize size(GetSize());
   tGUIPoint pos(GUIElementAbsolutePosition(const_cast<cGUIScrollBarElement*>(this)));

   if (m_scrollBarType == kGUIScrollBarHorizontal)
   {
      float threeHalfsHeight = size.height * 3 / 2;

      float rangeMinScreen = pos.x + threeHalfsHeight; // assumes width of thumb rect is size.height
      float rangeScreen = size.width - (size.height * 3); // assumes width of thumb rect is size.height

      float fracPos = (mousePos.x - rangeMinScreen) / rangeScreen;

      return m_rangeMin + FloatToInt(fracPos * (m_rangeMax - m_rangeMin));
   }
   else if (m_scrollBarType == kGUIScrollBarVertical)
   {
      tGUISizeType width3 = size.width * 3;

      // Check for out-of-bounds
      if (mousePos.x < (pos.x - width3)
         || mousePos.x > (pos.x + (size.width * 4))
         || mousePos.y < (pos.y - size.width)
         || mousePos.y > (pos.y + size.height + size.width))
      {
         return -1;
      }

      float threeHalfsWidth = width3 / 2;

      float rangeMinScreen = pos.y + threeHalfsWidth; // assumes height of thumb rect is size.width
      float rangeScreen = size.height - width3; // assumes height of thumb rect is size.width

      float fracPos = (mousePos.y - rangeMinScreen) / rangeScreen;

      int newPos = m_rangeMin + FloatToInt(fracPos * (m_rangeMax - m_rangeMin));
      if (newPos < m_rangeMin)
      {
         newPos = m_rangeMin;
      }
      else if (newPos > m_rangeMax)
      {
         newPos = m_rangeMax;
      }
      return newPos;
   }

   return -1;
}

///////////////////////////////////////

eGUIScrollBarPart cGUIScrollBarElement::GetHitPart(const tScreenPoint & point)
{
   static const eGUIScrollBarPart parts[] =
   {
      kGUIScrollBarPartButton1,
      kGUIScrollBarPartButton2,
      kGUIScrollBarPartTrack1,
      kGUIScrollBarPartTrack2,
      kGUIScrollBarPartThumb,
   };

   tGUIPoint pos = GUIElementAbsolutePosition(this);
   int ptx = FloatToInt(point.x - pos.x);
   int pty = FloatToInt(point.y - pos.y);

   for (int i = 0; i < _countof(parts); i++)
   {
      tGUIRect partRect;
      if (GetPartRect(parts[i], &partRect) == S_OK
         && partRect.PtInside(ptx, pty))
      {
         return parts[i];
      }
   }

   return kGUIScrollBarPartNone;
}

///////////////////////////////////////////////////////////////////////////////

static tResult GetScrollBarTypeFromString(const char * psz, eGUIScrollBarType * pSBType)
{
   if (psz == NULL || pSBType == NULL)
   {
      return E_POINTER;
   }

   eGUIScrollBarType scrollBarType = kGUIScrollBarVertical;
   if (_stricmp(psz, kValueVertical) == 0)
   {
      scrollBarType = kGUIScrollBarVertical;
   }
   else if (_stricmp(psz, kValueHorizontal) == 0)
   {
      scrollBarType = kGUIScrollBarHorizontal;
   }
   else
   {
      return E_INVALIDARG;
   }

   *pSBType = scrollBarType;
   return S_OK;
}

tResult GUIScrollBarElementCreateFromXml(const TiXmlElement * pXmlElement,
                                         IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementScrollBar) == 0)
      {
         eGUIScrollBarType scrollBarType;
         if (FAILED(GetScrollBarTypeFromString(pXmlElement->Attribute(kAttribType), &scrollBarType)))
         {
            return E_FAIL;
         }

         cAutoIPtr<IGUIScrollBarElement> pScrollBar = static_cast<IGUIScrollBarElement *>(
            new cGUIScrollBarElement(scrollBarType));
         if (!pScrollBar)
         {
            return E_OUTOFMEMORY;
         }

#define ScrollBarAttrib(attrib) \
   do { int value; if (pXmlElement->QueryIntAttribute(kAttrib##attrib, &value) == TIXML_SUCCESS) { \
   pScrollBar->Set##attrib(static_cast<uint>(value)); } } while (0)

         ScrollBarAttrib(ScrollPos);
         ScrollBarAttrib(LineSize);
         ScrollBarAttrib(PageSize);

#undef ScrollBarAttrib

         const char * pszAttrib = NULL;
         if ((pszAttrib = pXmlElement->Attribute(kAttribRange)) != NULL)
         {
            cTokenizer<int> tok;
            if (tok.Tokenize(pszAttrib) == 2)
            {
               pScrollBar->SetRange(tok.m_tokens[0], tok.m_tokens[1]);
            }
         }

         *ppElement = CTAddRef(pScrollBar);
         return S_OK;
      }
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(scrollbar, GUIScrollBarElementCreateFromXml);

///////////////////////////////////////////////////////////////////////////////
