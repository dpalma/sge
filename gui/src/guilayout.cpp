///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guilayout.h"

#include "guiapi.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "techmath.h"

#include <tinyxml.h>
#include <map>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(GUILayout);

#define LocalMsg(msg)                  DebugMsgEx(GUILayout,(msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(GUILayout,(msg),(a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(GUILayout,(msg),(a1),(a2))
#define LocalMsg3(msg,a1,a2,a3)        DebugMsgEx3(GUILayout,(msg),(a1),(a2),(a3))
#define LocalMsg4(msg,a1,a2,a3,a4)     DebugMsgEx4(GUILayout,(msg),(a1),(a2),(a3),(a4))
#define LocalMsg5(msg,a1,a2,a3,a4,a5)  DebugMsgEx5(GUILayout,(msg),(a1),(a2),(a3),(a4),(a5))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUILayout,(cond),(msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(GUILayout,(cond),(msg),(a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(GUILayout,(cond),(msg),(a1),(a2))
#define LocalMsgIf3(cond,msg,a1,a2,a3) DebugMsgIfEx3(GUILayout,(cond),(msg),(a1),(a2),(a3))

///////////////////////////////////////////////////////////////////////////////

static const int kHGapDefault = 0;
static const int kVGapDefault = 0;

///////////////////////////////////////////////////////////////////////////////

typedef std::map<cStr, tGUILayoutFactoryFn> tGUILayoutFactories;
static tGUILayoutFactories g_guiLayoutFactories;

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
      tGUILayoutFactories::iterator f = g_guiLayoutFactories.find(cStr(pszType));
      if (f != g_guiLayoutFactories.end())
      {
         return (*(f->second))(pXmlElement, ppLayout);
      }
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUILayoutRegister(const tChar * pszName, tGUILayoutFactoryFn pfn)
{
   if (pszName == NULL || pfn == NULL)
   {
      return E_POINTER;
   }

   g_guiLayoutFactories[cStr(pszName)] = pfn;
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void GUILayoutRegisterBuiltInTypes()
{
   GUILayoutRegister(kValueGrid, cGUIGridLayout::Create);
   GUILayoutRegister(kValueFlow, cGUIFlowLayout::Create);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIGridLayout
//

///////////////////////////////////////

tResult cGUIGridLayout::Create(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout)
{
   if (pXmlElement == NULL || ppLayout == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIGridLayout> pGridLayout;
   if (GUIGridLayoutCreate(&pGridLayout) == S_OK)
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

cGUIGridLayout::cGUIGridLayout()
 : m_hGap(kHGapDefault), 
   m_vGap(kVGapDefault),
   m_columns(2), 
   m_rows(2)
{
}

///////////////////////////////////////

cGUIGridLayout::cGUIGridLayout(uint columns, uint rows)
 : m_hGap(kHGapDefault), 
   m_vGap(kVGapDefault),
   m_columns(columns), 
   m_rows(rows)
{
}

///////////////////////////////////////

cGUIGridLayout::cGUIGridLayout(uint columns, uint rows, uint hGap, uint vGap)
 : m_hGap(hGap), 
   m_vGap(vGap),
   m_columns(columns), 
   m_rows(rows)
{
}

///////////////////////////////////////

cGUIGridLayout::~cGUIGridLayout()
{
}

///////////////////////////////////////

tResult cGUIGridLayout::Layout(IGUIElement * pParent)
{
   if (pParent == NULL)
   {
      return E_POINTER;
   }

   tGUIRect clientArea;
   if (pParent->GetClientArea(&clientArea) != S_OK)
   {
      return E_FAIL;
   }

   tGUISizeType cellWidth = clientArea.GetWidth() > 0 ? static_cast<tGUISizeType>((clientArea.GetWidth() - ((m_columns - 1) * m_hGap)) / m_columns) : 0;
   tGUISizeType cellHeight = clientArea.GetHeight() > 0 ? static_cast<tGUISizeType>((clientArea.GetHeight() - ((m_rows - 1) * m_vGap)) / m_rows) : 0;

   const tGUISize cellSize(cellWidth, cellHeight);

   LocalMsg2("Grid Layout (%d rows, %d columns)\n", m_rows, m_columns);
   LocalMsg2("   grid cell size = %.0f x %.0f\n", cellWidth, cellHeight);

   cAutoIPtr<IGUIElementEnum> pEnum;
   if (pParent->EnumChildren(&pEnum) == S_OK)
   {
      cAutoIPtr<IGUIElement> pChild;
      ulong count = 0;

      uint iRow = 0, iCol = 0;
      while ((pEnum->Next(1, &pChild, &count) == S_OK) && (count == 1))
      {
         if (pChild->IsVisible())
         {
            int x = clientArea.left + iCol * (Round(cellWidth) + m_hGap);
            int y = clientArea.top + iRow * (Round(cellHeight) + m_vGap);

            tGUIRect cellRect(x, y, x + Round(cellWidth), y + Round(cellHeight));

            GUIPlaceElement(cellRect, pChild);

#ifdef _DEBUG
            const tGUISize childSize(pChild->GetSize());
            LocalMsg5("   grid cell[%d][%d]: %p, %.0f x %.0f\n", iRow, iCol, pChild, childSize.width, childSize.height);
#endif
         }
#ifdef _DEBUG
         else
         {
            LocalMsg3("   grid cell[%d][%d]: %p is invisible\n", iRow, iCol, pChild);
         }
#endif

         // Invisible elements are allowed to occupy grid cells, so
         // this is outside of the if(visible) block above
         if (++iCol >= m_columns)
         {
            iCol = 0;
            if (++iRow >= m_rows)
            {
               break;
            }
         }

         SafeRelease(pChild);
         count = 0;
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayout::GetPreferredSize(IGUIElement * pParent, tGUISize * pSize)
{
   // A grid layout simply divides up whatever space is available.
   // It has no opinion what the size should be.
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIGridLayout::GetHGap(uint * pHGap)
{
   if (pHGap == NULL)
      return E_POINTER;
   *pHGap = m_hGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayout::SetHGap(uint hGap)
{
   m_hGap = hGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayout::GetVGap(uint * pVGap)
{
   if (pVGap == NULL)
      return E_POINTER;
   *pVGap = m_vGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayout::SetVGap(uint vGap)
{
   m_vGap = vGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayout::GetColumns(uint * pColumns)
{
   if (pColumns == NULL)
      return E_POINTER;
   *pColumns = m_columns;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayout::SetColumns(uint columns)
{
   m_columns = columns;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayout::GetRows(uint * pRows)
{
   if (pRows == NULL)
      return E_POINTER;
   *pRows = m_rows;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIGridLayout::SetRows(uint rows)
{
   m_rows = rows;
   return S_OK;
}

///////////////////////////////////////

tResult GUIGridLayoutCreate(IGUIGridLayout * * ppLayout)
{
   if (ppLayout == NULL)
      return E_POINTER;
   cAutoIPtr<IGUIGridLayout> pLayout = new cGUIGridLayout;
   if (!pLayout)
      return E_OUTOFMEMORY;
   *ppLayout = CTAddRef(pLayout);
   return S_OK;
}

///////////////////////////////////////

tResult GUIGridLayoutCreate(uint columns, uint rows, IGUIGridLayout * * ppLayout)
{
   if (ppLayout == NULL)
      return E_POINTER;
   cAutoIPtr<IGUIGridLayout> pLayout = new cGUIGridLayout(columns, rows);
   if (!pLayout)
      return E_OUTOFMEMORY;
   *ppLayout = CTAddRef(pLayout);
   return S_OK;
}

///////////////////////////////////////

tResult GUIGridLayoutCreate(uint columns, uint rows, uint hGap, uint vGap, IGUIGridLayout * * ppLayout)
{
   if (ppLayout == NULL)
      return E_POINTER;
   cAutoIPtr<IGUIGridLayout> pLayout = new cGUIGridLayout(columns, rows, hGap, vGap);
   if (!pLayout)
      return E_OUTOFMEMORY;
   *ppLayout = CTAddRef(pLayout);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFlowLayout
//

///////////////////////////////////////

GUI_API IGUILayoutManager * GUIFlowLayoutCreate()
{
   return static_cast<IGUILayoutManager*>(new cGUIFlowLayout);
}

///////////////////////////////////////

tResult cGUIFlowLayout::Create(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout)
{
   if (pXmlElement == NULL || ppLayout == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIFlowLayout> pFlowLayout(new cGUIFlowLayout);
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

cGUIFlowLayout::cGUIFlowLayout()
 : m_hGap(kHGapDefault), 
   m_vGap(kVGapDefault)
{
}

///////////////////////////////////////

cGUIFlowLayout::cGUIFlowLayout(uint hGap, uint vGap)
 : m_hGap(hGap), 
   m_vGap(vGap)
{
}

///////////////////////////////////////

cGUIFlowLayout::~cGUIFlowLayout()
{
}

///////////////////////////////////////

tResult cGUIFlowLayout::Layout(IGUIElement * pParent)
{
   if (pParent == NULL)
   {
      return E_POINTER;
   }

   tGUIRect clientArea;
   if (pParent->GetClientArea(&clientArea) != S_OK)
   {
      return E_FAIL;
   }

   const tGUISize clientSize(
      static_cast<tGUISizeType>(clientArea.GetWidth()),
      static_cast<tGUISizeType>(clientArea.GetHeight()));

   cAutoIPtr<IGUIElementEnum> pEnum;
   if (pParent->EnumChildren(&pEnum) == S_OK)
   {
      int x = clientArea.left;
      int y = clientArea.top;

      int nChildrenThisRow = 0;

      cAutoIPtr<IGUIElement> pChild;
      ulong count = 0;

      while ((pEnum->Next(1, &pChild, &count) == S_OK) && (count == 1))
      {
         if (pChild->IsVisible())
         {
            tGUISize childSize(pChild->GetSize());
            tGUIRect rect(x, y, Round(x + childSize.width), Round(y + childSize.height));
            GUIPlaceElement(rect, pChild);
            x += Round(childSize.width + m_hGap);
            nChildrenThisRow++;
            if (x >= clientArea.right)
            {
               y += Round(childSize.height + m_vGap);
               x = clientArea.left;
               nChildrenThisRow = 0;
            }
         }

         SafeRelease(pChild);
         count = 0;
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIFlowLayout::GetPreferredSize(IGUIElement * pParent, tGUISize * pSize)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cGUIFlowLayout::GetHGap(uint * pHGap)
{
   if (pHGap == NULL)
      return E_POINTER;
   *pHGap = m_hGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIFlowLayout::SetHGap(uint hGap)
{
   m_hGap = hGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIFlowLayout::GetVGap(uint * pVGap)
{
   if (pVGap == NULL)
      return E_POINTER;
   *pVGap = m_vGap;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIFlowLayout::SetVGap(uint vGap)
{
   m_vGap = vGap;
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

tResult GUILayoutElementCreate(const TiXmlElement * pXmlElement,
                               IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL && pParent != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementLayout) == 0)
      {
         cAutoIPtr<IGUIContainerElement> pContainer;
         if (pParent->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
         {
            cAutoIPtr<IGUILayoutManager> pLayout;
            if (GUILayoutManagerCreate(pXmlElement, &pLayout) == S_OK)
            {
               if (pContainer->SetLayout(pLayout) != S_OK)
               {
                  WarnMsg("Error creating layout manager\n");
               }
            }

            return S_FALSE;
         }
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

AUTOREGISTER_GUIELEMENTFACTORYFN(layout, GUILayoutElementCreate);

///////////////////////////////////////////////////////////////////////////////
