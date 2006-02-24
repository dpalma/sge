///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipagelayout.h"
#include "guielementtools.h"
#include "guistyleapi.h"

#include "hashtabletem.h"

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(GUIPageLayout);

#define LocalMsg(msg)                  DebugMsgEx(GUIPageLayout,msg)
#define LocalMsg1(msg,a)               DebugMsgEx1(GUIPageLayout,msg,(a))
#define LocalMsg2(msg,a,b)             DebugMsgEx2(GUIPageLayout,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)           DebugMsgEx3(GUIPageLayout,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)         DebugMsgEx4(GUIPageLayout,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUIPageLayout,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(GUIPageLayout,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(GUIPageLayout,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(GUIPageLayout,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(GUIPageLayout,(cond),msg,(a),(b),(c),(d))


///////////////////////////////////////////////////////////////////////////////

static tGUIString GUIElementIdentify(IGUIElement * pElement)
{
   tGUIString type = GUIElementType(pElement);
   if (pElement != NULL)
   {
      tGUIString id;
      if (pElement->GetId(&id) == S_OK)
      {
         tGUIString temp;
         return tGUIString(Sprintf(&temp, _T("{%s: id=%s}"), type.c_str(), id.c_str()));
      }
   }
   return type;
}


///////////////////////////////////////////////////////////////////////////////

static tResult GUIElementSize(IGUIElement * pElement, IGUIElementRenderer * pRenderer,
                              const tGUISize & baseSize, tGUISize * pSize)
{
   tGUISize preferredSize;
   bool bHavePreferredSize = (pRenderer->GetPreferredSize(pElement, &preferredSize) == S_OK);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      tGUISize styleSize(0,0);

      if (GUIStyleWidth(pStyle, baseSize.width, &styleSize.width) != S_OK)
      {
         if (bHavePreferredSize)
         {
            styleSize.width = preferredSize.width;
         }
         else
         {
            return S_FALSE;
         }
      }

      if (GUIStyleHeight(pStyle, baseSize.height, &styleSize.height) != S_OK)
      {
         if (bHavePreferredSize)
         {
            styleSize.height = preferredSize.height;
         }
         else
         {
            return S_FALSE;
         }
      }

      *pSize = styleSize;
      return S_OK;
   }
   else if (bHavePreferredSize)
   {
      *pSize = preferredSize;
      return S_OK;
   }

   return S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPageLayoutFlow
//

///////////////////////////////////////

cGUIPageLayoutFlow::cGUIPageLayoutFlow()
{
}

///////////////////////////////////////

cGUIPageLayoutFlow::cGUIPageLayoutFlow(const tGUIRect & rect)
 : m_rect(rect)
 , m_pos(static_cast<float>(rect.left), static_cast<float>(rect.top))
 , m_rowHeight(0)
{
}

///////////////////////////////////////

cGUIPageLayoutFlow::cGUIPageLayoutFlow(const cGUIPageLayoutFlow & other)
 : m_rect(other.m_rect)
 , m_pos(other.m_pos)
 , m_rowHeight(other.m_rowHeight)
{
}

///////////////////////////////////////

cGUIPageLayoutFlow::~cGUIPageLayoutFlow()
{
}

///////////////////////////////////////

const cGUIPageLayoutFlow & cGUIPageLayoutFlow::operator =(const cGUIPageLayoutFlow & other)
{
   m_rect = other.m_rect;
   m_pos = other.m_pos;
   m_rowHeight = other.m_rowHeight;
   return *this;
}

///////////////////////////////////////

void cGUIPageLayoutFlow::PlaceElement(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      tGUISize elementSize(pElement->GetSize());

      tGUIPoint elementPos(m_pos);
      bool bUpdateFlowPos = true;

      cAutoIPtr<IGUIStyle> pStyle;
      if (pElement->GetStyle(&pStyle) == S_OK)
      {
         uint placement;
         if (pStyle->GetPlacement(&placement) == S_OK)
         {
            if (placement == kGUIPlaceRelative)
            {
               // TODO
            }
            else if (placement == kGUIPlaceAbsolute)
            {
               uint align;
               if (pStyle->GetAlignment(&align) == S_OK)
               {
                  if (align == kGUIAlignLeft)
                  {
                     elementPos.x = static_cast<float>(m_rect.left);
                  }
                  else if (align == kGUIAlignRight)
                  {
                     elementPos.x = static_cast<float>(m_rect.right - elementSize.width);
                  }
                  else if (align == kGUIAlignCenter)
                  {
                     elementPos.x = static_cast<float>(m_rect.left + ((m_rect.GetWidth() - elementSize.width) / 2));
                  }
                  // If position changed, don't update the flow layout's current position
                  if (elementPos.x != m_pos.x)
                  {
                     bUpdateFlowPos = false;
                  }
               }

               uint vertAlign;
               if (pStyle->GetVerticalAlignment(&vertAlign) == S_OK)
               {
                  if (vertAlign == kGUIVertAlignTop)
                  {
                     elementPos.y = static_cast<float>(m_rect.top);
                  }
                  else if (vertAlign == kGUIVertAlignBottom)
                  {
                     elementPos.y = m_rect.bottom - elementSize.height;
                  }
                  else if (vertAlign == kGUIVertAlignCenter)
                  {
                     elementPos.y = m_rect.top + ((m_rect.GetHeight() - elementSize.height) / 2);
                  }
                  // If position changed, don't update the flow layout's current position
                  if (elementPos.y != m_pos.y)
                  {
                     bUpdateFlowPos = false;
                  }
               }
            }
         }
      }

      pElement->SetPosition(elementPos);

      if (bUpdateFlowPos)
      {
         m_pos.x += elementSize.width;
         if (elementSize.height > m_rowHeight)
         {
            m_rowHeight = elementSize.height;
         }
         if (m_pos.x >= m_rect.right)
         {
            m_pos.x = static_cast<float>(m_rect.left);
            m_pos.y += m_rowHeight;
            m_rowHeight = 0;
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPageLayout
//

///////////////////////////////////////

cGUIPageLayout::cGUIPageLayout(const tGUIRect & rect)
 : m_topLevelRect(rect)
 , m_topLevelSize(static_cast<tGUISizeType>(rect.GetWidth()), static_cast<tGUISizeType>(rect.GetHeight()))
 , m_topLevelFlow(rect)
{
}

///////////////////////////////////////

cGUIPageLayout::cGUIPageLayout(const cGUIPageLayout & other)
 : m_topLevelRect(other.m_topLevelRect)
 , m_topLevelSize(other.m_topLevelSize)
 , m_topLevelFlow(other.m_topLevelFlow)
{
}

///////////////////////////////////////

cGUIPageLayout::~cGUIPageLayout()
{
   while (!m_layoutQueue.empty())
   {
      cAutoIPtr<IGUIContainerElement> pContainer(m_layoutQueue.front());
      m_layoutQueue.pop();

      cAutoIPtr<IGUILayoutManager> pLayout;
      if (pContainer->GetLayout(&pLayout) == S_OK)
      {
         pLayout->Layout(pContainer);
      }
   }

   tFlowTable::iterator iter = m_flowTable.begin();
   for (; iter != m_flowTable.end(); iter++)
   {
      delete iter->second;
   }
}

///////////////////////////////////////

tResult cGUIPageLayout::operator ()(IGUIElement * pElement, IGUIElementRenderer * pRenderer, void *)
{
   Assert(pElement != NULL);
   Assert(pRenderer != NULL);

   tGUISize parentSize(0,0);
   cAutoIPtr<IGUIElement> pParent;
   if (pElement->GetParent(&pParent) == S_OK)
   {
      parentSize = pParent->GetSize();
   }
   else
   {
      parentSize = m_topLevelSize;
   }

   tGUISize elementSize;
   if (GUIElementSize(pElement, pRenderer, parentSize, &elementSize) != S_OK)
   {
      return S_FALSE;
   }

   if (AlmostEqual(elementSize.width, 0) || AlmostEqual(elementSize.height, 0))
   {
      return S_FALSE;
   }

   pElement->SetSize(elementSize);

   tGUIRect clientArea(0,0,0,0);
   pRenderer->ComputeClientArea(pElement, &clientArea);

   pElement->SetClientArea(clientArea);

   if (!pParent)
   {
      LocalMsg1("Top level element: %s\n", GUIElementIdentify(pElement).c_str());
      m_topLevelFlow.PlaceElement(pElement);
   }
   else
   {
      LocalMsg1("Child element: %s\n", GUIElementIdentify(pElement).c_str());
      tFlowTable::iterator iter = m_flowTable.find(tGUIElementKey(CTAddRef(pParent)));
      if (iter != m_flowTable.end())
      {
         iter->second->PlaceElement(pElement);
      }
   }

   cAutoIPtr<IGUIContainerElement> pContainer;
   if (pElement->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
   {
      cAutoIPtr<IGUILayoutManager> pLayout;
      if (pContainer->GetLayout(&pLayout) == S_OK)
      {
         m_layoutQueue.push(CTAddRef(pContainer));
      }
      else
      {
         cGUIPageLayoutFlow * pFlow = new cGUIPageLayoutFlow(clientArea);
         if (pFlow != NULL)
         {
            m_flowTable.insert(tGUIElementKey(CTAddRef(pElement)), pFlow);
         }
      }
   }

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
