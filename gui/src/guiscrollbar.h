///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUISCROLLBAR_H
#define INCLUDED_GUISCROLLBAR_H

#include "guielementbase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIScrollBarElement
//

class cGUIScrollBarElement : public cComObject<cGUIElementBase<IGUIScrollBarElement>, &IID_IGUIScrollBarElement>
{
public:
   cGUIScrollBarElement(eGUIScrollBarType scrollBarType);
   ~cGUIScrollBarElement();

   // Over-rides
   virtual tResult OnEvent(IGUIEvent * pEvent);

   // IGUIScrollBarElement methods
   virtual eGUIScrollBarType GetScrollBarType();

   virtual eGUIScrollBarPart GetArmedPart();
   virtual eGUIScrollBarPart GetMouseOverPart();

   virtual tResult GetPartRect(eGUIScrollBarPart part, tGUIRect * pRect);

   virtual tResult GetRange(int * pRangeMin, int * pRangeMax);
   virtual tResult SetRange(int rangeMin, int rangeMax);

   virtual tResult GetScrollPos(int * pScrollPos);
   virtual tResult SetScrollPos(int scrollPos);

   virtual tResult GetLineSize(int * pLineSize);
   virtual tResult SetLineSize(int lineSize);

   virtual tResult GetPageSize(int * pPageSize);
   virtual tResult SetPageSize(int pageSize);

protected:
   int DetermineScrollPos(const tScreenPoint & mousePos) const;
   eGUIScrollBarPart GetHitPart(const tScreenPoint & point);

private:
   eGUIScrollBarType m_scrollBarType;
   eGUIScrollBarPart m_armedPart, m_mouseOverPart;
   int m_rangeMin, m_rangeMax;
   int m_scrollPos;
   int m_lineSize;
   int m_pageSize;

   int m_dragOffset;
   int m_dragStartScrollPos;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUISCROLLBAR_H
