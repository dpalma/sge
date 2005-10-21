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
   virtual void SetSize(const tGUISize & size);
   virtual tResult OnEvent(IGUIEvent * pEvent);

   // IGUIScrollBarElement methods
   virtual eGUIScrollBarType GetScrollBarType();

   virtual eGUIScrollBarPart GetArmedPart();
   virtual eGUIScrollBarPart GetMouseOverPart();

   virtual tResult GetPartRect(eGUIScrollBarPart part, tGUIRect * pRect);

   virtual tResult GetRange(uint * pRangeMin, uint * pRangeMax);
   virtual tResult SetRange(uint rangeMin, uint rangeMax);

   virtual tResult GetPosition(uint * pPosition);
   virtual tResult SetPosition(uint position);

   virtual tResult GetLineSize(uint * pLineSize);
   virtual tResult SetLineSize(uint lineSize);

   virtual tResult GetPageSize(uint * pPageSize);
   virtual tResult SetPageSize(uint pageSize);

protected:
   eGUIScrollBarPart GetHitPart(const tGUIPoint & point);

private:
   eGUIScrollBarType m_scrollBarType;
   eGUIScrollBarPart m_armedPart, m_mouseOverPart;
   uint m_rangeMin, m_rangeMax;
   uint m_position;
   uint m_lineSize;
   uint m_pageSize;

   uint m_preDragPosition;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIScrollBarElementFactory
//

class cGUIScrollBarElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUISCROLLBAR_H
