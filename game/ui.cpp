///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ui.h"
#include "uievent.h"
#include "uiparse.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIComponent
//

///////////////////////////////////////

cUIComponent::cUIComponent()
 : m_x(0), m_y(0),
   m_size(0,0),
   m_pParent(NULL),
   m_bAcceptsFocus(true),
   m_bHasFocus(false),
   m_bVisible(true)
{
}

///////////////////////////////////////

cUIComponent::~cUIComponent()
{
}

///////////////////////////////////////

cUIComponent * cUIComponent::HitTest(const cUIPoint & point)
{
   return GetScreenRect().PtInside(point) ? this : NULL;
}

///////////////////////////////////////

bool cUIComponent::OnEvent(const cUIEvent * pEvent)
{
   std::list<IUIEventListener *>::iterator iter;
   for (iter = m_listeners.begin(); iter != m_listeners.end(); iter++)
   {
      if ((*iter)->OnEvent(pEvent))
      {
         return true;
      }
   }
   return false;
}

///////////////////////////////////////
// is the given component a descendant of this one?

bool cUIComponent::IsChild(const cUIComponent * pComponent) const
{
   while (pComponent != NULL)
   {
      if (pComponent->GetParent() == this)
         return true;
      pComponent = pComponent->GetParent();
   }
   return false;
}

///////////////////////////////////////

cUIRect cUIComponent::GetScreenRect() const
{
   cUIPoint point(m_x, m_y);
   const cUIComponent * pParent = GetParent();
   while (pParent != NULL)
   {
      point += pParent->GetPos();
      pParent = pParent->GetParent();
   }
   return cUIRect(point, GetSize());
}

///////////////////////////////////////

tResult cUIComponent::AddListener(IUIEventListener * pListener)
{
   return add_interface(m_listeners, pListener) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cUIComponent::RemoveListener(IUIEventListener * pListener)
{
   return remove_interface(m_listeners, pListener) ? S_OK : S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUILayoutManager
//

class cUILayoutManager
{
public:
   virtual ~cUILayoutManager() = 0;

   virtual void Layout(cUIContainer * pContainer) = 0;
   virtual cUISize GetPreferredSize(cUIContainer * pContainer) const = 0;
};

///////////////////////////////////////

cUILayoutManager::~cUILayoutManager()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIFlowLayoutManager
//

class cUIFlowLayoutManager : public cUILayoutManager
{
   enum { kHorzGapDefault = 5, kVertGapDefault = 5 };

public:
   cUIFlowLayoutManager(float horzGap = kHorzGapDefault,
                        float vertGap = kVertGapDefault);

   virtual void Layout(cUIContainer * pContainer);
   virtual cUISize GetPreferredSize(cUIContainer * pContainer) const;

   float GetHorzGap() const { return m_horzGap; }
   float GetVertGap() const { return m_vertGap; }

private:
   float m_horzGap, m_vertGap;
};

///////////////////////////////////////

cUIFlowLayoutManager::cUIFlowLayoutManager(float horzGap, float vertGap)
 : m_horzGap(horzGap), m_vertGap(vertGap)
{
}

///////////////////////////////////////

void cUIFlowLayoutManager::Layout(cUIContainer * pContainer)
{
   float x = pContainer->GetMargins().left;
   float y = pContainer->GetMargins().top;
   float curRowHeight = 0;

   tUIComponentList components;
   pContainer->GetComponents(&components);

   float widthAvail = pContainer->GetSize().width -
      pContainer->GetMargins().right - 
      pContainer->GetMargins().left;

   float heightAvail = pContainer->GetSize().height -
      pContainer->GetMargins().top - 
      pContainer->GetMargins().bottom;

   float rightMargin = pContainer->GetSize().width -
      pContainer->GetMargins().right;

   tUIComponentList::iterator iter;
   for (iter = components.begin(); iter != components.end(); iter++)
   {
      cUIComponent * pComponent = (*iter);

      cUISize size = pComponent->GetPreferredSize();

      if (size.width > widthAvail)
         size.width = widthAvail;
      if (size.height > heightAvail)
         size.height = heightAvail;

      if (x + (size.width + GetHorzGap()) >= rightMargin)
      {
         y += curRowHeight + GetVertGap();
         x = pContainer->GetMargins().left;
         curRowHeight = size.height;
      }
      else
      {
         if (size.height > curRowHeight)
            curRowHeight = size.height;
      }

      pComponent->SetPos(x, y);
      pComponent->SetSize(size);

      x += size.width + GetHorzGap();
   }
}

///////////////////////////////////////

cUISize cUIFlowLayoutManager::GetPreferredSize(cUIContainer * pContainer) const
{
   return cUISize(0,0);
}

///////////////////////////////////////

cUILayoutManager * UIFlowLayoutManagerCreate()
{
   return new cUIFlowLayoutManager;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIGridLayoutManager
//

class cUIGridLayoutManager : public cUILayoutManager
{
   enum { kHorzGapDefault = 5, kVertGapDefault = 5 };

public:
   cUIGridLayoutManager(int nRows, int nCols);

   virtual void Layout(cUIContainer * pContainer);
   virtual cUISize GetPreferredSize(cUIContainer * pContainer) const;

   int GetRows() const { return m_nRows; }
   int GetCols() const { return m_nCols; }

private:
   int m_nRows, m_nCols;
};

///////////////////////////////////////

cUIGridLayoutManager::cUIGridLayoutManager(int nRows, int nCols)
 : m_nRows(nRows), m_nCols(nCols)
{
}

///////////////////////////////////////

// @TODO (dpalma 9/3/2003): This function will lay out components in a grid
// but columns will not always be aligned.
void cUIGridLayoutManager::Layout(cUIContainer * pContainer)
{
   tUIComponentList components;
   pContainer->GetComponents(&components);

   int iRow = 0, iCol = 0;
   float rowHeight = 0;
   float x = pContainer->GetMargins().left;
   float y = pContainer->GetMargins().top;

   tUIComponentList::iterator iter;
   for (iter = components.begin(); iter != components.end(); iter++)
   {
      cUIComponent * pComponent = (*iter);

      cUISize size = pComponent->GetPreferredSize();

      if (size.height > rowHeight)
         rowHeight = size.height;

      pComponent->SetPos(x, y);
      pComponent->SetSize(size);

      x += size.width + kHorzGapDefault;

      if (++iCol >= GetCols())
      {
         iCol = 0;
         y += rowHeight + kVertGapDefault;
         rowHeight = 0;
         x = pContainer->GetMargins().left;
         if (++iRow >= GetRows())
            break;
      }
   }
}

///////////////////////////////////////

cUISize cUIGridLayoutManager::GetPreferredSize(cUIContainer * pContainer) const
{
   int iRow = 0, iCol = 0;
   float curRowWidth = 0, maxRowWidth = 0, rowHeight = 0, totalHeight = 0;

   tUIComponentList components;
   pContainer->GetComponents(&components);

   tUIComponentList::iterator iter;
   for (iter = components.begin(); iter != components.end(); iter++)
   {
      cUISize size = (*iter)->GetPreferredSize();

      if (size.height > rowHeight)
         rowHeight = size.height;

      curRowWidth += size.width;

      if (++iCol >= GetCols())
      {
         totalHeight += rowHeight;
         rowHeight = 0;
         if (curRowWidth > maxRowWidth)
            maxRowWidth = curRowWidth;
         curRowWidth = 0;
         iCol = 0;
         if (++iRow >= GetRows())
            break;
      }
   }

   const cUIMargins margins = pContainer->GetMargins();

   maxRowWidth += ((GetCols() - 1) * kHorzGapDefault) + margins.right + margins.left;
   totalHeight += ((GetRows() - 1) * kVertGapDefault) + margins.top + margins.bottom;

   return cUISize(maxRowWidth, totalHeight);
}

///////////////////////////////////////

cUILayoutManager * UIGridLayoutManagerCreate(int nRows, int nCols)
{
   return new cUIGridLayoutManager(nRows, nCols);
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIContainerBase
//

///////////////////////////////////////

cUIContainerBase::cUIContainerBase()
 : cUIComponent()
{
}

///////////////////////////////////////

cUIContainerBase::~cUIContainerBase()
{
   RemoveAll();
}

///////////////////////////////////////

void cUIContainerBase::Render(IRenderDevice * pRenderDevice)
{
   if (IsVisible())
   {
      tUIComponentList::iterator iter;
      for (iter = m_children.begin(); iter != m_children.end(); iter++)
      {
         if ((*iter)->IsVisible())
         {
            (*iter)->Render(pRenderDevice);
         }
      }
   }
}

///////////////////////////////////////

cUIComponent * cUIContainerBase::HitTest(const cUIPoint & point)
{
   if (cUIComponent::HitTest(point))
   {
      tUIComponentList::reverse_iterator iter;
      for (iter = m_children.rbegin(); iter != m_children.rend(); iter++)
      {
         if ((*iter)->IsVisible())
         {
            cUIComponent * pHit = (*iter)->HitTest(point);
            if (pHit != NULL)
               return pHit;
         }
      }
      return this;
   }
   return NULL;
}

///////////////////////////////////////

bool cUIContainerBase::OnEvent(const cUIEvent * pEvent)
{
   Assert(pEvent != NULL);
   if (pEvent->code == kEventDestroy)
   {
      tUIComponentList::iterator iter = FindChild(pEvent->pSrc);
      if (iter != m_children.end())
      {
         RemoveComponent(pEvent->pSrc);
      }
   }
   return cUIComponent::OnEvent(pEvent);
}

///////////////////////////////////////

void cUIContainerBase::AddComponent(cUIComponent * pComponent)
{
   if (pComponent != NULL)
   {
      m_children.push_back(pComponent);
      pComponent->SetParent(this);
   }
}

///////////////////////////////////////

bool cUIContainerBase::RemoveComponent(cUIComponent * pComponent)
{
   Assert(pComponent != NULL);
   tUIComponentList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if ((*iter) == pComponent)
      {
         // it is now up to the caller of this method to delete the component
         m_children.erase(iter);
         return true;
      }
   }
   return false;
}

///////////////////////////////////////

void cUIContainerBase::GetComponents(tUIComponentList * pComponents)
{
   Assert(pComponents != NULL);
   pComponents->clear();
   tUIComponentList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
      pComponents->push_back(*iter);
}

///////////////////////////////////////

void cUIContainerBase::RemoveAll()
{
   tUIComponentList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
      delete *iter;
   m_children.clear();
}

///////////////////////////////////////

cUIComponent * cUIContainerBase::FindChild(const char * pszId)
{
   Assert(pszId != NULL);
   if (pszId == NULL)
      return NULL;
   tUIComponentList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (stricmp((*iter)->GetId(), pszId) == 0)
         return (*iter);
   }
   return NULL;
}

///////////////////////////////////////

tUIComponentList::iterator cUIContainerBase::FindChild(cUIComponent * pComponent)
{
   tUIComponentList::iterator iter;
   for (iter = m_children.begin(); iter != m_children.end(); iter++)
   {
      if (*iter == pComponent)
         return iter;
   }
   return m_children.end();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIContainer
//

///////////////////////////////////////

cUIContainer::cUIContainer()
 : cUIContainerBase(),
   m_margins(0,0,0,0),
   m_pLayoutManager(NULL)
{
}

///////////////////////////////////////

cUIContainer::~cUIContainer()
{
   SetLayoutManager(NULL);
}

///////////////////////////////////////

void cUIContainer::Layout()
{
   if (AccessLayoutManager() == NULL)
   {
      //DebugMsg("WARNING: No layout manager in cUIContainer::Layout()... Creating a default\n");
      SetLayoutManager(UIFlowLayoutManagerCreate());
   }

   // if no explicit size given, have the layout manager calculate one
   if (GetSize() == cUISize(0,0))
   {
      SetSize(AccessLayoutManager()->GetPreferredSize(this));
   }

   AccessLayoutManager()->Layout(this);
}

///////////////////////////////////////

void cUIContainer::SetLayoutManager(cUILayoutManager * pLayoutManager)
{
   delete m_pLayoutManager;
   m_pLayoutManager = pLayoutManager;
}

///////////////////////////////////////////////////////////////////////////////
