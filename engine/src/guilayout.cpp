///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilayout.h"
#include "guielementtools.h"
#include "guistrings.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static tResult GUIGridLayoutManagerCreate(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout)
{
   if (pXmlElement == NULL || ppLayout == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   cAutoIPtr<IGUIGridLayoutManager> pGridLayout;
   if ((result = GUIGridLayoutManagerCreate(&pGridLayout)) == S_OK)
   {
      int value;

      if (pXmlElement->QueryIntAttribute(kAttribHgap, &value) == TIXML_SUCCESS)
      {
         pGridLayout->SetHGap(value);
      }

      if (pXmlElement->QueryIntAttribute(kAttribVgap, &value) == TIXML_SUCCESS)
      {
         pGridLayout->SetVGap(value);
      }

      if (pXmlElement->QueryIntAttribute(kAttribColumns, &value) == TIXML_SUCCESS)
      {
         pGridLayout->SetColumns(value);
      }

      if (pXmlElement->QueryIntAttribute(kAttribRows, &value) == TIXML_SUCCESS)
      {
         pGridLayout->SetRows(value);
      }

      *ppLayout = CTAddRef(pGridLayout);
      return S_OK;
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUILayoutManagerCreate(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout)
{
   if (pXmlElement == NULL || ppLayout == NULL)
   {
      return E_POINTER;
   }

   if (stricmp(pXmlElement->Value(), kAttribLayout) != 0)
   {
      return E_INVALIDARG;
   }

   const char * pszType;
   if ((pszType = pXmlElement->Attribute(kAttribType)) != NULL)
   {
      if (stricmp(pszType, kValueGrid) == 0)
      {
         return GUIGridLayoutManagerCreate(pXmlElement, ppLayout);
      }
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIEvent
//

static const int kHGapDefault = 5;
static const int kVGapDefault = 5;

///////////////////////////////////////

cGUIGridLayoutManager::cGUIGridLayoutManager()
 : m_hGap(kHGapDefault), 
   m_vGap(kVGapDefault),
   m_columns(2), 
   m_rows(2)
{
}

///////////////////////////////////////

cGUIGridLayoutManager::cGUIGridLayoutManager(uint columns, uint rows)
 : m_hGap(kHGapDefault), 
   m_vGap(kVGapDefault),
   m_columns(columns), 
   m_rows(rows)
{
}

///////////////////////////////////////

cGUIGridLayoutManager::cGUIGridLayoutManager(uint columns, uint rows, uint hGap, uint vGap)
 : m_hGap(hGap), 
   m_vGap(vGap),
   m_columns(columns), 
   m_rows(rows)
{
}

///////////////////////////////////////

cGUIGridLayoutManager::~cGUIGridLayoutManager()
{
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::Layout(IGUIContainerElement * pContainer)
{
   if (pContainer == NULL)
   {
      return E_POINTER;
   }

   tGUISize size = pContainer->GetSize();

   tGUIInsets insets = {0};
   if (pContainer->GetInsets(&insets) == S_OK)
   {
      size.width -= (insets.left + insets.right);
      size.height -= (insets.top + insets.bottom);
   }

   float cellWidth = size.width > 0 ? (size.width - ((m_columns - 1) * m_hGap)) / m_columns : 0;
   float cellHeight = size.height > 0 ? (size.height - ((m_rows - 1) * m_vGap)) / m_rows : 0;

   cAutoIPtr<IGUIElementEnum> pEnum;
   if (pContainer->GetElements(&pEnum) == S_OK)
   {
      cAutoIPtr<IGUIElement> pChild;
      ulong count = 0;

      uint iRow = 0, iCol = 0;
      while ((pEnum->Next(1, &pChild, &count) == S_OK) && (count == 1))
      {
         if (pChild->IsVisible())
         {
            float x = insets.left + iCol * (cellWidth + m_hGap);
            float y = insets.top + iRow * (cellHeight + m_vGap);

            tGUIRect cellRect(x, y, x + cellWidth, y + cellHeight);

            GUISizeElement(cellRect, pChild);
            GUIPlaceElement(cellRect, pChild);

            if (++iCol >= m_columns)
            {
               iCol = 0;
               if (++iRow >= m_rows)
               {
                  break;
               }
            }
         }

         SafeRelease(pChild);
         count = 0;
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::GetPreferredSize(IGUIContainerElement * pContainer, 
                                                tGUISize * pSize)
{
   if (pContainer == NULL || pSize == NULL)
   {
      return E_POINTER;
   }

   uint iRow = 0, iCol = 0;
   tGUISizeType curRowWidth = 0;
   tGUISizeType maxRowWidth = 0;
   tGUISizeType rowHeight = 0;
   tGUISizeType totalHeight = 0;

   cAutoIPtr<IGUIElementEnum> pEnum;
   if (pContainer->GetElements(&pEnum) == S_OK)
   {
      cAutoIPtr<IGUIElement> pChild;
      ulong count = 0;

      while ((pEnum->Next(1, &pChild, &count) == S_OK) && (count == 1))
      {
         if (pChild->IsVisible())
         {
            cAutoIPtr<IGUIElementRenderer> pChildRenderer;
            if (pChild->GetRenderer(&pChildRenderer) == S_OK)
            {
               tGUISize childSize = pChildRenderer->GetPreferredSize(pChild);

               if (childSize.height > rowHeight)
               {
                  rowHeight = childSize.height;
               }

               curRowWidth += childSize.width;

               if (++iCol >= m_columns)
               {
                  totalHeight += rowHeight;
                  rowHeight = 0;

                  if (curRowWidth > maxRowWidth)
                  {
                     maxRowWidth = curRowWidth;
                  }
                  curRowWidth = 0;

                  iCol = 0;
                  if (++iRow >= m_rows)
                  {
                     break;
                  }
               }
            }
         }

         SafeRelease(pChild);
         count = 0;
      }
   }

   tGUIInsets insets;
   if (pContainer->GetInsets(&insets) == S_OK)
   {
      if (m_columns > 0)
      {
         maxRowWidth += ((m_columns - 1) * m_hGap) + insets.right + insets.left;
      }

      if (m_rows > 0)
      {
         totalHeight += ((m_rows - 1) * m_vGap) + insets.top + insets.bottom;
      }
   }

   Assert(pSize != NULL);
   *pSize = tGUISize(maxRowWidth, totalHeight);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::GetHGap(uint * pHGap)
{
   if (pHGap == NULL)
      return E_POINTER;
   *pHGap = m_hGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::SetHGap(uint hGap)
{
   m_hGap = hGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::GetVGap(uint * pVGap)
{
   if (pVGap == NULL)
      return E_POINTER;
   *pVGap = m_vGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::SetVGap(uint vGap)
{
   m_vGap = vGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::GetColumns(uint * pColumns)
{
   if (pColumns == NULL)
      return E_POINTER;
   *pColumns = m_columns;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::SetColumns(uint columns)
{
   m_columns = columns;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::GetRows(uint * pRows)
{
   if (pRows == NULL)
      return E_POINTER;
   *pRows = m_rows;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayoutManager::SetRows(uint rows)
{
   m_rows = rows;
   return S_OK;
}

///////////////////////////////////////

tResult GUIGridLayoutManagerCreate(IGUIGridLayoutManager * * ppLayout)
{
   if (ppLayout == NULL)
      return E_POINTER;
   cAutoIPtr<IGUIGridLayoutManager> pLayout = new cGUIGridLayoutManager;
   if (!pLayout)
      return E_OUTOFMEMORY;
   *ppLayout = CTAddRef(pLayout);
   return S_OK;
}

///////////////////////////////////////

tResult GUIGridLayoutManagerCreate(uint columns, uint rows, IGUIGridLayoutManager * * ppLayout)
{
   if (ppLayout == NULL)
      return E_POINTER;
   cAutoIPtr<IGUIGridLayoutManager> pLayout = new cGUIGridLayoutManager(columns, rows);
   if (!pLayout)
      return E_OUTOFMEMORY;
   *ppLayout = CTAddRef(pLayout);
   return S_OK;
}

///////////////////////////////////////

tResult GUIGridLayoutManagerCreate(uint columns, uint rows, uint hGap, uint vGap, IGUIGridLayoutManager * * ppLayout)
{
   if (ppLayout == NULL)
      return E_POINTER;
   cAutoIPtr<IGUIGridLayoutManager> pLayout = new cGUIGridLayoutManager(columns, rows, hGap, vGap);
   if (!pLayout)
      return E_OUTOFMEMORY;
   *ppLayout = CTAddRef(pLayout);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
