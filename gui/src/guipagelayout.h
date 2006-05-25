///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIPAGELAYOUT_H
#define INCLUDED_GUIPAGELAYOUT_H

#include "guiapi.h"
#include "guielementapi.h"

#include "techhash.h"
#include "hashtable.h"

#include <queue>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////

template <typename INTRFC>
class cInterfaceHashKey
{
public:
   cInterfaceHashKey()
   {
   }

   explicit cInterfaceHashKey(INTRFC * pInterface)
    : m_pInterface(pInterface) // Assume ownership of existing ref (caller must AddRef)
   {
   }

   INTRFC * AccessInterface() const
   {
      return m_pInterface.AccessPointer();
   }

private:
   cAutoIPtr<INTRFC> m_pInterface;
};

template <typename INTRFC>
class cInterfaceHashFunction
{
public:
   static uint Hash(const cInterfaceHashKey<INTRFC> & a, uint initHash = 0xDEADBEEF)
   {
      cAutoIPtr<IUnknown> pUnk;
      if (a.AccessInterface()->QueryInterface(IID_IUnknown, (void**)&pUnk) == S_OK)
      {
         IUnknown * pUnkRaw = (IUnknown*)pUnk;
         return hash((byte*)&pUnkRaw, sizeof(IUnknown*), initHash);
      }
      return initHash;
   }

   static bool Equal(const cInterfaceHashKey<INTRFC> & a, const cInterfaceHashKey<INTRFC> & b)
   {
      return CTIsSameObject(a.AccessInterface(), b.AccessInterface());
   }
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPageLayoutFlow
//

class cGUIPageLayoutFlow
{
public:
   cGUIPageLayoutFlow();
   cGUIPageLayoutFlow(const tGUIRect & rect, bool bLTR = true);
   cGUIPageLayoutFlow(const cGUIPageLayoutFlow & other);
   ~cGUIPageLayoutFlow();

   const cGUIPageLayoutFlow & operator =(const cGUIPageLayoutFlow & other);

   void PlaceElement(IGUIElement * pElement);

private:
   tGUIRect m_rect;
   bool m_bLTR;
   tGUIPoint m_pos;
   tGUISizeType m_rowHeight;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPageLayout
//
// A functor class for use with GUIElementRenderLoop to lay out 
// elements on a page

class cGUIPageLayout
{
public:
   explicit cGUIPageLayout(const tGUIRect & rect);
   cGUIPageLayout(const tGUIRect & rect, IGUIElement * pRequester, uint options);
   cGUIPageLayout(const cGUIPageLayout &);
   ~cGUIPageLayout();

   tResult operator ()(IGUIElement * pElement, IGUIElementRenderer * pRenderer, void *);

private:
   IGUIElement * AccessRequester() { return m_pRequester; }

   bool IsOptionSet(uint option) const { return ((m_options & option) == option); }

   const tGUIRect m_topLevelRect;
   const tGUISize m_topLevelSize;
   cGUIPageLayoutFlow m_topLevelFlow;

   typedef cInterfaceHashKey<IGUIElement> tGUIElementKey;
   typedef cHashTable<tGUIElementKey, cGUIPageLayoutFlow *,
                      cInterfaceHashFunction<IGUIElement> > tFlowTable;
   tFlowTable m_flowTable;

   std::queue<IGUIContainerElement*> m_layoutQueue;

   cAutoIPtr<IGUIElement> m_pRequester; // element requesting layout

   uint m_options;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIPAGELAYOUT_H
