///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUITITLEBAR_H
#define INCLUDED_GUITITLEBAR_H

#include "guielementbase.h"
#include "gui/guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITitleBarElement
//

typedef cGUIElementBase<IGUITitleBarElement> tGUITitleBarBase;

class cGUITitleBarElement : public cComObject2<tGUITitleBarBase, &IID_IGUITitleBarElement,
                                               IMPLEMENTS(IGUIDragSource)>
{
public:
   cGUITitleBarElement();
   ~cGUITitleBarElement();

   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual tResult QueryContinueDrag(const sInputEvent * pInputEvent);

   virtual tResult GetTitle(tGUIString * pTitle);
   virtual tResult SetTitle(const tGUIChar * pszTitle);

private:
   tGUIRect GetAbsoluteRect();

   bool m_bDragging;
   tScreenPoint m_dragOffset;

   tGUIString m_title;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUITITLEBAR_H
