///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUILAYOUT_H
#define INCLUDED_GUILAYOUT_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIGridLayout
//

class cGUIGridLayout : public cComObject<IMPLEMENTS(IGUIGridLayout)>
{
public:
   static tResult Create(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout);

   cGUIGridLayout();
   cGUIGridLayout(uint columns, uint rows);
   cGUIGridLayout(uint columns, uint rows, uint hGap, uint vGap);
   ~cGUIGridLayout();

   virtual tResult Layout(IGUIElement * pParent);
   virtual tResult GetPreferredSize(IGUIElement * pParent, tGUISize * pSize);

   virtual tResult GetHGap(uint * pHGap);
   virtual tResult SetHGap(uint hGap);

   virtual tResult GetVGap(uint * pVGap);
   virtual tResult SetVGap(uint vGap);

   virtual tResult GetColumns(uint * pColumns);
   virtual tResult SetColumns(uint columns);

   virtual tResult GetRows(uint * pRows);
   virtual tResult SetRows(uint rows);

private:
   uint m_hGap, m_vGap;
   uint m_columns, m_rows;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFlowLayout
//

class cGUIFlowLayout : public cComObject<IMPLEMENTS(IGUIFlowLayout)>
{
public:
   static tResult Create(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout);

   cGUIFlowLayout();
   cGUIFlowLayout(uint hGap, uint vGap);
   ~cGUIFlowLayout();

   virtual tResult Layout(IGUIElement * pParent);
   virtual tResult GetPreferredSize(IGUIElement * pParent, tGUISize * pSize);

   virtual tResult GetHGap(uint * pHGap);
   virtual tResult SetHGap(uint hGap);

   virtual tResult GetVGap(uint * pVGap);
   virtual tResult SetVGap(uint vGap);

private:
   uint m_hGap, m_vGap;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUILAYOUT_H
