///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UI_H
#define INCLUDED_UI_H

#include "uitypes.h"
#include "comtools.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

class cUIEvent;
class cUIComponent;
class cUILayoutManager;
class cUIContainerBase;
class cUIContainer;

F_DECLARE_INTERFACE(IRenderDevice);

///////////////////////////////////////////////////////////////////////////////

cUIPoint UIGetMousePos();

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIComponent
//

enum eUIComponentFlags
{
   kUICF_NONE = 0,
   kUICF_Focussed = 1 << 0,
   kUICF_Visible = 1 << 1,
};

#define kUICF_Default (kUICF_Visible)

class cUIComponent
{
   cUIComponent(const cUIComponent &);
   const cUIComponent & operator=(const cUIComponent &);

public:
   cUIComponent(uint flags = kUICF_Default);
   virtual ~cUIComponent() = 0;

   virtual void Render(IRenderDevice * pRenderDevice) = 0;

   virtual cUIPoint GetPos() const;
   virtual void SetPos(float x, float y);

   virtual const char * GetId() const;
   virtual void SetId(const char * pszId);

   virtual void SetSize(const cUISize & size);
   virtual cUISize GetSize() const;
   virtual cUISize GetPreferredSize() const;

   virtual cUIComponent * HitTest(const cUIPoint & point);

   virtual bool OnEvent(const cUIEvent * pEvent);

   cUIComponent * GetParent();
   const cUIComponent * GetParent() const;

   // is 'pComponent' a descendant of this one?
   bool IsChild(const cUIComponent * pComponent) const;

   cUIRect GetScreenRect() const;

   void SetInternalFlags(uint flags, uint mask);
   bool TestInternalFlags(uint flags) const;

   bool AcceptsFocus() const { return m_bAcceptsFocus; }

protected:
   friend class cUIContainerBase;
   friend class cUIContainer;

   void SetParent(cUIComponent * pParent);

   bool IsVisible() const;

   void NoFocus() { m_bAcceptsFocus = false; }

private:
   float m_x, m_y;
   cUISize m_size;
   cUIComponent * m_pParent;
   cUIString m_id;
   uint m_flags;
   bool m_bAcceptsFocus;
};

///////////////////////////////////////

inline cUIPoint cUIComponent::GetPos() const
{
   return cUIPoint(m_x, m_y);
}

///////////////////////////////////////

inline void cUIComponent::SetPos(float x, float y)
{
   m_x = x;
   m_y = y;
}

///////////////////////////////////////

inline const char * cUIComponent::GetId() const
{
   return m_id.c_str();
}

///////////////////////////////////////

inline void cUIComponent::SetId(const char * pszId)
{
   Assert(pszId != NULL);
   if (pszId != NULL)
      m_id = pszId;
}

///////////////////////////////////////

inline void cUIComponent::SetSize(const cUISize & size)
{
   m_size = size;
}

///////////////////////////////////////

inline cUISize cUIComponent::GetSize() const
{
   return m_size;
}

///////////////////////////////////////

inline cUISize cUIComponent::GetPreferredSize() const
{
   return GetSize();
}

///////////////////////////////////////

inline cUIComponent * cUIComponent::GetParent()
{
   return m_pParent;
}

///////////////////////////////////////

inline const cUIComponent * cUIComponent::GetParent() const
{
   return m_pParent;
}

///////////////////////////////////////

inline void cUIComponent::SetParent(cUIComponent * pParent)
{
   m_pParent = pParent;
}

///////////////////////////////////////

inline bool cUIComponent::IsVisible() const
{
   return TestInternalFlags(kUICF_Visible);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIContainerBase
//

typedef std::list<cUIComponent *> tUIComponentList;

class cUIContainerBase : public cUIComponent
{
public:
   cUIContainerBase(uint flags = kUICF_Default);
   virtual ~cUIContainerBase() = 0;

   ////////////////////////////////////

   virtual void Render(IRenderDevice * pRenderDevice);
   virtual cUIComponent * HitTest(const cUIPoint & point);
   virtual bool OnEvent(const cUIEvent * pEvent);

   ////////////////////////////////////

   void AddComponent(cUIComponent * pComponent);
   bool RemoveComponent(cUIComponent * pComponent);
   void GetComponents(tUIComponentList * pComponents);
   void RemoveAll();

   cUIComponent * FindChild(const char * pszId);

protected:
   tUIComponentList::iterator FindChild(cUIComponent * pComponent);

private:
   // deliberately un-implemented and private copy constructor and assignment operator
   cUIContainerBase(const cUIContainerBase &);
   const cUIContainerBase & operator=(const cUIContainerBase &);

   tUIComponentList m_children;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIContainer
//

class cUIContainer : public cUIContainerBase
{
public:
   cUIContainer(uint flags = kUICF_Default);
   virtual ~cUIContainer();

   ////////////////////////////////////

   const cUIMargins & GetMargins() const;
   void SetMargins(const cUIMargins & margins);

   void Layout();
   void SetLayoutManager(cUILayoutManager * pLayoutManager);

private:
   cUILayoutManager * AccessLayoutManager();

   // deliberately un-implemented and private copy constructor and assignment operator
   cUIContainer(const cUIContainer &);
   const cUIContainer & operator=(const cUIContainer &);

   cUIMargins m_margins;
   cUILayoutManager * m_pLayoutManager;
};

///////////////////////////////////////

inline const cUIMargins & cUIContainer::GetMargins() const
{
   return m_margins;
}

///////////////////////////////////////

inline void cUIContainer::SetMargins(const cUIMargins & margins)
{
   m_margins = margins;
}

///////////////////////////////////////

inline cUILayoutManager * cUIContainer::AccessLayoutManager()
{
   return m_pLayoutManager;
}

///////////////////////////////////////

cUILayoutManager * UIFlowLayoutManagerCreate();
cUILayoutManager * UIGridLayoutManagerCreate(int nRows, int nCols);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_UI_H
