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
// CLASS: cGUIGridLayoutManager
//

class cGUIGridLayoutManager : public cComObject<IMPLEMENTS(IGUIGridLayoutManager)>
{
public:
   cGUIGridLayoutManager();
   cGUIGridLayoutManager(uint columns, uint rows);
   cGUIGridLayoutManager(uint columns, uint rows, uint hGap, uint vGap);
   ~cGUIGridLayoutManager();

   virtual tResult Layout(IGUIContainerElement * pContainer);
   virtual tResult GetPreferredSize(IGUIContainerElement * pContainer, tGUISize * pSize);

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

#endif // !INCLUDED_GUILAYOUT_H
