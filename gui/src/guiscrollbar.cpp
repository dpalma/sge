///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guiscrollbar.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "globalobj.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIScrollBarElement
//

///////////////////////////////////////

cGUIScrollBarElement::cGUIScrollBarElement(eGUIScrollBarType scrollBarType)
 : m_scrollBarType(scrollBarType)
 , m_armedPart(kGUIScrollBarPartNone)
 , m_mouseOverPart(kGUIScrollBarPartNone)
 , m_rangeMin(0)
 , m_rangeMax(100)
 , m_position(0)
 , m_lineSize(1)
 , m_pageSize(5)
{
}

///////////////////////////////////////

cGUIScrollBarElement::~cGUIScrollBarElement()
{
}

///////////////////////////////////////

void cGUIScrollBarElement::SetSize(const tGUISize & size)
{
   cGUIElementBase<IGUIScrollBarElement>::SetSize(size);
}

///////////////////////////////////////

tResult cGUIScrollBarElement::OnEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tResult result = S_OK; // allow event processing to continue

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   tGUIPoint point;
   Verify(pEvent->GetMousePosition(&point) == S_OK);

   if (eventCode == kGUIEventDragStart)
   {
      Verify(pEvent->SetCancelBubble(true) == S_OK);

      m_armedPart = m_mouseOverPart;
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
               m_position -= m_lineSize;
               if (m_position < m_rangeMin)
               {
                  m_position = m_rangeMin;
               }
               break;
            }
            case kGUIScrollBarPartButton2:
            {
               m_position += m_lineSize;
               if (m_position > m_rangeMax)
               {
                  m_position = m_rangeMax;
               }
               break;
            }
            case kGUIScrollBarPartTrack1:
            {
               m_position -= m_pageSize;
               if (m_position < m_rangeMin)
               {
                  m_position = m_rangeMin;
               }
               break;
            }
            case kGUIScrollBarPartTrack2:
            {
               m_position += m_pageSize;
               if (m_position > m_rangeMax)
               {
                  m_position = m_rangeMax;
               }
               break;
            }
            case kGUIScrollBarPartThumb:
            {
               break;
            }
         }
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

   tGUISize size = GetSize();
   int width = Round(size.width);
   int height = Round(size.height);

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
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::GetRange(uint * pRangeMin, uint * pRangeMax)
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

tResult cGUIScrollBarElement::SetRange(uint rangeMin, uint rangeMax)
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

tResult cGUIScrollBarElement::GetPosition(uint * pPosition)
{
   if (pPosition == NULL)
   {
      return E_POINTER;
   }
   *pPosition = m_position;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::SetPosition(uint position)
{
   if (position < m_rangeMin)
   {
      position = m_rangeMin;
   }
   if (position > m_rangeMax)
   {
      position = m_rangeMax;
   }
   m_position = position;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::GetLineSize(uint * pLineSize)
{
   if (pLineSize == NULL)
   {
      return E_POINTER;
   }
   *pLineSize = m_lineSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::SetLineSize(uint lineSize)
{
   if (lineSize > (m_rangeMin - m_rangeMax))
   {
      return E_INVALIDARG;
   }
   m_lineSize = lineSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::GetPageSize(uint * pPageSize)
{
   if (pPageSize == NULL)
   {
      return E_POINTER;
   }
   *pPageSize = m_pageSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIScrollBarElement::SetPageSize(uint pageSize)
{
   if (pageSize > (m_rangeMin - m_rangeMax))
   {
      return E_INVALIDARG;
   }
   m_pageSize = pageSize;
   return S_OK;
}

///////////////////////////////////////

eGUIScrollBarPart cGUIScrollBarElement::GetHitPart(const tGUIPoint & point)
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
   int ptx = Round(point.x - pos.x);
   int pty = Round(point.y - pos.y);

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
//
// CLASS: cGUIScrollBarElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(scrollbar, cGUIScrollBarElementFactory);

tResult cGUIScrollBarElementFactory::CreateElement(const TiXmlElement * pXmlElement, 
                                                   IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementScrollBar) == 0)
      {
         const char * pszAttrib;

         eGUIScrollBarType scrollBarType = kGUIScrollBarVertical;
         if ((pszAttrib = pXmlElement->Attribute(kAttribType)) != NULL)
         {
            if (stricmp(pszAttrib, kValueVertical) == 0)
            {
               scrollBarType = kGUIScrollBarVertical;
            }
            else if (stricmp(pszAttrib, kValueHorizontal) == 0)
            {
               scrollBarType = kGUIScrollBarHorizontal;
            }
            else
            {
               return E_INVALIDARG;
            }
         }

         cAutoIPtr<IGUIScrollBarElement> pScrollBar = static_cast<IGUIScrollBarElement *>(
            new cGUIScrollBarElement(scrollBarType));
         if (!pScrollBar)
         {
            return E_OUTOFMEMORY;
         }
         GUIElementStandardAttributes(pXmlElement, pScrollBar);

#define ScrollBarAttrib(attrib) \
   do { if ((pszAttrib = pXmlElement->Attribute(kAttrib##attrib)) != NULL) { \
   uint value; if (sscanf(pszAttrib, "%d", &value) == 1) pScrollBar->Set##attrib(value); } } while (0)

         ScrollBarAttrib(Position);
         ScrollBarAttrib(LineSize);
         ScrollBarAttrib(PageSize);

#undef ScrollBarAttrib

         if ((pszAttrib = pXmlElement->Attribute(kAttribRange)) != NULL)
         {
            float rangeValues[2];
            if (cStr(pszAttrib).ParseTuple(rangeValues, _countof(rangeValues)) == 2)
            {
               pScrollBar->SetRange(Round(rangeValues[0]), Round(rangeValues[1]));
            }
         }

         *ppElement = CTAddRef(pScrollBar);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUIScrollBarElement *>(new cGUIScrollBarElement(kGUIScrollBarVertical));
      return (*ppElement != NULL) ? S_OK : E_OUTOFMEMORY;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
