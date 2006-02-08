///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTAINERBASE_H
#define INCLUDED_GUICONTAINERBASE_H

#include "guielementbase.h"

#include <list>
#include <algorithm>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIContainerBase
//
// Template base class that implements the IGUIContainerElement methods.
//
// The template parameter INTRFC is a derivative of IGUIContainerElement
// (e.g., IGUIPanelElement or IGUIDialogElement)

template <typename INTRFC>
class cGUIContainerBase : public cGUIElementBase<INTRFC>
{
   typedef cGUIElementBase<INTRFC> tBaseClass;

public:
   cGUIContainerBase(IGUILayoutManager * pLayout = NULL);
   ~cGUIContainerBase();

   ////////////////////////////////////
   // IGUIElement over-rides

   virtual tResult EnumChildren(IGUIElementEnum * * ppElements);

   ////////////////////////////////////
   // IGUIContainerElement methods

   virtual tResult AddElement(IGUIElement * pElement);
   virtual tResult RemoveElement(IGUIElement * pElement);
   virtual tResult HasElement(IGUIElement * pElement) const;
   virtual tResult GetElement(const tChar * pszId, IGUIElement * * ppElement) const;

   virtual tResult GetLayout(IGUILayoutManager * * ppLayout);
   virtual tResult SetLayout(IGUILayoutManager * pLayout);

protected:
   template <typename F>
   void ForEachElement(F f)
   {
      std::for_each(m_children.begin(), m_children.end(), f);
   }

private:
   typedef std::list<IGUIElement *> tGUIElementList;
   tGUIElementList m_children;

   cAutoIPtr<IGUILayoutManager> m_pLayout;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUICONTAINERBASE_H
