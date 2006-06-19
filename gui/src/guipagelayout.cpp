///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipagelayout.h"
#include "guielementtools.h"
#include "guistyleapi.h"

#include "hashtabletem.h"

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://www.w3.org/TR/WD-positioning-19970819


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
         return tGUIString(Sprintf(&temp, _T("%s (id=%s)"), type.c_str(), id.c_str()));
      }
   }
   return type;
}


///////////////////////////////////////////////////////////////////////////////

static tResult GUIElementSize(IGUIElement * pElement, IGUIElementRenderer * pRenderer,
                              const tGUISize & parentSize, tGUISize * pSize)
{
   tGUISize preferredSize;
   bool bHavePreferredSize = (pRenderer->GetPreferredSize(pElement, parentSize, &preferredSize) == S_OK);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      tGUISize styleSize(0,0);

      if (GUIStyleWidth(pStyle, parentSize.width, &styleSize.width) != S_OK)
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

      if (GUIStyleHeight(pStyle, parentSize.height, &styleSize.height) != S_OK)
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

cGUIPageLayoutFlow::cGUIPageLayoutFlow(const tGUIRect & rect, bool bLTR /*=true*/)
 : m_rect(rect)
 , m_bLTR(bLTR)
 , m_pos(bLTR ? rect.GetTopLeft<float>() : rect.GetTopRight<float>())
 , m_rowHeight(0)
{
}

///////////////////////////////////////

cGUIPageLayoutFlow::cGUIPageLayoutFlow(const cGUIPageLayoutFlow & other)
 : m_rect(other.m_rect)
 , m_bLTR(other.m_bLTR)
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
   m_bLTR = other.m_bLTR;
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

      // TODO: Handle elements that are too big for the page. Truncate them or something.

      tGUIPoint elementPos(m_pos);
      if (!m_bLTR)
      {
         elementPos.x -= elementSize.width;
      }
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
         bool bNextRow = false;
         if (m_bLTR)
         {
            m_pos.x += elementSize.width;
            if (m_pos.x >= m_rect.right)
            {
               m_pos.x = static_cast<float>(m_rect.left);
               bNextRow = true;
            }
         }
         else
         {
            m_pos.x -= elementSize.width;
            if (m_pos.x <= m_rect.left)
            {
               m_pos.x = static_cast<float>(m_rect.right);
               bNextRow = true;
            }
         }
         if (elementSize.height > m_rowHeight)
         {
            m_rowHeight = elementSize.height;
         }
         if (bNextRow)
         {
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
 , m_options(kGUILayoutDefault)
{
}

///////////////////////////////////////

cGUIPageLayout::cGUIPageLayout(const tGUIRect & rect, IGUIElement * pRequester, uint options)
 : m_topLevelRect(rect)
 , m_topLevelSize(static_cast<tGUISizeType>(rect.GetWidth()), static_cast<tGUISizeType>(rect.GetHeight()))
 , m_topLevelFlow(rect)
 , m_pRequester(CTAddRef(pRequester))
 , m_options(options)
{
}

///////////////////////////////////////

cGUIPageLayout::cGUIPageLayout(const cGUIPageLayout & other)
 : m_topLevelRect(other.m_topLevelRect)
 , m_topLevelSize(other.m_topLevelSize)
 , m_topLevelFlow(other.m_topLevelFlow)
 , m_pRequester(other.m_pRequester)
 , m_options(other.m_options)
{
}

///////////////////////////////////////

cGUIPageLayout::~cGUIPageLayout()
{
   while (!m_layoutQueue.empty())
   {
      cAutoIPtr<IGUIContainerElement> pContainer(m_layoutQueue.front().first);
      tGUIRect containerRect(m_layoutQueue.front().second);
      m_layoutQueue.pop();

      cAutoIPtr<IGUILayoutManager> pLayout;
      if (pContainer->GetLayout(&pLayout) == S_OK)
      {
         pLayout->Layout(pContainer, containerRect);
      }
   }

   tFlowTable::iterator iter = m_flowTable.begin();
   for (; iter != m_flowTable.end(); iter++)
   {
      delete iter->second;
   }
}

///////////////////////////////////////

tResult cGUIPageLayout::operator ()(IGUIElement * pElement, IGUIElementRenderer * pRenderer,
                                    const tGUIPoint & /*position*/, void *)
{
   Assert(pElement != NULL);
   Assert(pRenderer != NULL);

   if (!!m_pRequester)
   {
      if (!CTIsSameObject(pElement, m_pRequester) && !IsDescendant(m_pRequester, pElement))
      {
         // Return S_OK to continue processing children
         return S_OK;
      }
   }

   tGUISize elementSize(0,0);

   if (!IsOptionSet(kGUILayoutNoSize))
   {
      tGUISize parentSize(0,0);
      cAutoIPtr<IGUIElement> pParent;
      if (pElement->GetParent(&pParent) == S_OK)
      {
         tGUIRect parentRect;
         tFlowTable::iterator f = m_flowTable.find(tGUIElementKey(CTAddRef(pParent)));
         if (f != m_flowTable.end())
         {
            parentRect = f->second->GetRect();
            parentSize = tGUISize(static_cast<tGUISizeType>(parentRect.GetWidth()),
                                  static_cast<tGUISizeType>(parentRect.GetHeight()));
         }
         else if (SUCCEEDED(pParent->GetClientArea(&parentRect)))
         {
            parentSize = tGUISize(static_cast<tGUISizeType>(parentRect.GetWidth()),
                                  static_cast<tGUISizeType>(parentRect.GetHeight()));
         }
      }
      else
      {
         parentSize = m_topLevelSize;
      }

      if (GUIElementSize(pElement, pRenderer, parentSize, &elementSize) != S_OK)
      {
         return S_FALSE;
      }

      pElement->SetSize(elementSize);
   }
   else
   {
      elementSize = pElement->GetSize();
   }

   if (!IsOptionSet(kGUILayoutNoMove))
   {
      cAutoIPtr<IGUIElement> pParent;
      if (pElement->GetParent(&pParent) == S_OK)
      {
         tFlowTable::iterator iter = m_flowTable.find(tGUIElementKey(CTAddRef(pParent)));
         if (iter != m_flowTable.end())
         {
            iter->second->PlaceElement(pElement);
         }
         LocalMsg3("Placed child element %s at (%.0f, %.0f)\n", GUIElementIdentify(pElement).c_str(),
            pElement->GetPosition().x, pElement->GetPosition().y);
      }
      else
      {
         m_topLevelFlow.PlaceElement(pElement);
         LocalMsg3("Placed top level element %s at (%.0f, %.0f)\n", GUIElementIdentify(pElement).c_str(),
            pElement->GetPosition().x, pElement->GetPosition().y);
      }
   }

   tGUIRect clientArea(0, 0, FloatToInt(elementSize.width), FloatToInt(elementSize.height));

   // Allow renderer to allocate space for borders
   pRenderer->AllocateBorderSpace(pElement, &clientArea);

   tResult computeClientAreaResult = pElement->ComputeClientArea(pRenderer, &clientArea);
   if (SUCCEEDED(computeClientAreaResult))
   {
      pElement->SetClientArea(clientArea);
      if (computeClientAreaResult == S_FALSE)
      {
         return S_FALSE;
      }
   }

   cAutoIPtr<IGUIContainerElement> pContainer;
   if (pElement->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
   {
      cAutoIPtr<IGUILayoutManager> pLayout;
      if (pContainer->GetLayout(&pLayout) == S_OK)
      {
         LocalMsg1("Element %s has a layout manager\n", GUIElementIdentify(pElement).c_str());
         m_layoutQueue.push(std::make_pair(static_cast<IGUIContainerElement*>(CTAddRef(pContainer)), clientArea));
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
