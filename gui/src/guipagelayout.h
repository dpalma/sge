///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIPAGELAYOUT_H
#define INCLUDED_GUIPAGELAYOUT_H

#include "guiapi.h"
#include "guielementapi.h"

#include <queue>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPageLayoutFlow
//

class cGUIPageLayoutFlow
{
public:
   cGUIPageLayoutFlow();
   cGUIPageLayoutFlow(const tGUIRect & rect);
   cGUIPageLayoutFlow(const cGUIPageLayoutFlow & other);
   ~cGUIPageLayoutFlow();

   const cGUIPageLayoutFlow & operator =(const cGUIPageLayoutFlow & other);

   void PlaceElement(IGUIElement * pElement);

private:
   tGUIRect m_rect;
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
   cGUIPageLayout(const cGUIPageLayout &);
   ~cGUIPageLayout();

   tResult operator ()(IGUIElement * pElement, IGUIElementRenderer * pRenderer, void *);

private:
   const tGUIRect m_topLevelRect;
   const tGUISize m_topLevelSize;
   cGUIPageLayoutFlow m_topLevelFlow;

   std::queue<IGUIContainerElement*> m_layoutQueue;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIPAGELAYOUT_H
