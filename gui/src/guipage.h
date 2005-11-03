///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIPAGE_H
#define INCLUDED_GUIPAGE_H

#include "guielementbase.h"

#include "connptimpl.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

typedef std::list<IGUIElement *> tGUIElementList;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPage
//

class cGUIPage
{
public:
   cGUIPage(const tGUIElementList * pElements);
   ~cGUIPage();

   void SetOverlay(bool bIsOverlay) { m_bIsOverlay = bIsOverlay; }
   bool IsOverlay() const { return m_bIsOverlay; }

   void Clear();

   tResult AddElement(IGUIElement * pElement);
   tResult RemoveElement(IGUIElement * pElement);

   size_t CountElements() const;
   tResult HasElement(IGUIElement * pElement) const;
   tResult GetElement(const tChar * pszId, IGUIElement * * ppElement);

   tResult GetActiveModalDialog(IGUIDialogElement * * ppDialog);

   void UpdateLayout(const tGUIRect & rect);
   void Render(IGUIRenderDevice * pRenderDevice);

   tResult GetHitElements(const tGUIPoint & point, tGUIElementList * pElements) const;

   void RunScripts();

   tGUIElementList::const_iterator BeginElements() const { return m_elements.begin(); }
   tGUIElementList::const_iterator EndElements() const { return m_elements.end(); }

private:
   tGUIElementList m_elements;
   bool m_bUpdateLayout;
   bool m_bIsOverlay;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIPAGE_H
