///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUICONTAINERBASE_H
#define INCLUDED_GUICONTAINERBASE_H

#include "guiapi.h"

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
class cGUIContainerBase : public INTRFC
{
public:
   cGUIContainerBase();
   ~cGUIContainerBase();

   ////////////////////////////////////
   // IGUIContainerElement methods

   virtual tResult AddElement(IGUIElement * pElement);
   virtual tResult RemoveElement(IGUIElement * pElement);
   virtual tResult GetElements(IGUIElementEnum * * ppElements);
   virtual tResult HasElement(IGUIElement * pElement) const;

protected:
   template <typename F>
   void ForEachElement(F f)
   {
      std::for_each(m_children.begin(), m_children.end(), f);
   }

private:
   typedef std::list<IGUIElement *> tGUIElementList;
   tGUIElementList m_children;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUICONTAINERBASE_H
