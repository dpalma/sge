///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilayout.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "techmath.h"

#include <tinyxml.h>
#include <map>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static const int kHGapDefault = 0;
static const int kVGapDefault = 0;

///////////////////////////////////////////////////////////////////////////////

typedef std::map<cStr, tGUILayoutManagerFactoryFn> tGUILayoutManagerFactories;
static tGUILayoutManagerFactories g_guiLayoutManagerFactories;

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
      tGUILayoutManagerFactories::iterator f = g_guiLayoutManagerFactories.find(cStr(pszType));
      if (f != g_guiLayoutManagerFactories.end())
      {
         return (*(f->second))(pXmlElement, ppLayout);
      }
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUILayoutManagerRegister(const tChar * pszName, tGUILayoutManagerFactoryFn pfn)
{
   if (pszName == NULL || pfn == NULL)
   {
      return E_POINTER;
   }

   g_guiLayoutManagerFactories[cStr(pszName)] = pfn;
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void GUILayoutManagerRegisterBuiltInTypes()
{
   GUILayoutManagerRegister(kValueGrid, cGUIGridLayoutManager::Create);
   GUILayoutManagerRegister(kValueFlow, cGUIFlowLayoutManager::Create);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIGridLayoutManager
//

///////////////////////////////////////

tResult cGUIGridLayoutManager::Create(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout)
{
   if (pXmlElement == NULL || ppLayout == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIGridLayoutManager> pGridLayout;
   if (GUIGridLayoutManagerCreate(&pGridLayout) == S_OK)
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

   return E_FAIL;
}

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

   int cellWidth = size.width > 0 ? Round((size.width - ((m_columns - 1) * m_hGap)) / m_columns) : 0;
   int cellHeight = size.height > 0 ? Round((size.height - ((m_rows - 1) * m_vGap)) / m_rows) : 0;

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
            int x = insets.left + iCol * (cellWidth + m_hGap);
            int y = insets.top + iRow * (cellHeight + m_vGap);

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
   // A grid layout simply divides up whatever space is available.
   // It has no opinion what the size should be.
   return E_NOTIMPL;
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
//
// CLASS: cGUIFlowLayoutManager
//

///////////////////////////////////////

tResult cGUIFlowLayoutManager::Create(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout)
{
   if (pXmlElement == NULL || ppLayout == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIFlowLayoutManager> pFlowLayout(new cGUIFlowLayoutManager);
   if (!pFlowLayout)
   {
      return E_OUTOFMEMORY;
   }

   int value;
   if (pXmlElement->QueryIntAttribute(kAttribHgap, &value) == TIXML_SUCCESS)
   {
      pFlowLayout->SetHGap(value);
   }

   if (pXmlElement->QueryIntAttribute(kAttribVgap, &value) == TIXML_SUCCESS)
   {
      pFlowLayout->SetVGap(value);
   }

   *ppLayout = CTAddRef(pFlowLayout);
   return S_OK;
}

///////////////////////////////////////

cGUIFlowLayoutManager::cGUIFlowLayoutManager()
 : m_hGap(kHGapDefault), 
   m_vGap(kVGapDefault)
{
}

///////////////////////////////////////

cGUIFlowLayoutManager::cGUIFlowLayoutManager(uint hGap, uint vGap)
 : m_hGap(hGap), 
   m_vGap(vGap)
{
}

///////////////////////////////////////

cGUIFlowLayoutManager::~cGUIFlowLayoutManager()
{
}

///////////////////////////////////////

tResult cGUIFlowLayoutManager::Layout(IGUIContainerElement * pContainer)
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

   int leftSide = insets.left;
   int rightSide = Round(leftSide + size.width);

   cAutoIPtr<IGUIElementEnum> pEnum;
   if (pContainer->GetElements(&pEnum) == S_OK)
   {
      int x = leftSide;
      int y = insets.top;

      int nChildrenThisRow = 0;

      cAutoIPtr<IGUIElement> pChild;
      ulong count = 0;

      while ((pEnum->Next(1, &pChild, &count) == S_OK) && (count == 1))
      {
         if (pChild->IsVisible())
         {
            tGUISize childSize;
            if (GUIElementSizeFromStyle(pChild, size, &childSize) == S_OK)
            {
               pChild->SetSize(childSize);
               tGUIRect rect(x, y, Round(x + childSize.width), Round(y + childSize.height));
               GUIPlaceElement(rect, pChild);
               x += Round(childSize.width + m_hGap);
               nChildrenThisRow++;
               if (x >= rightSide)
               {
                  y += Round(childSize.height + m_vGap);
                  x = leftSide;
                  nChildrenThisRow = 0;
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

tResult cGUIFlowLayoutManager::GetPreferredSize(IGUIContainerElement * pContainer, 
                                                tGUISize * pSize)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIFlowLayoutManager::GetHGap(uint * pHGap)
{
   if (pHGap == NULL)
      return E_POINTER;
   *pHGap = m_hGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIFlowLayoutManager::SetHGap(uint hGap)
{
   m_hGap = hGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIFlowLayoutManager::GetVGap(uint * pVGap)
{
   if (pVGap == NULL)
      return E_POINTER;
   *pVGap = m_vGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIFlowLayoutManager::SetVGap(uint vGap)
{
   m_vGap = vGap;
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
