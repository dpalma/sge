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

F_DECLARE_INTERFACE(IGUIRenderDevice);

typedef std::list<IGUIElement *> tGUIElementList;

class TiXmlDocument;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPage
//

class cGUIPage
{
   cGUIPage(const tGUIElementList * pElements);
public:
   ~cGUIPage();

   static tResult Create(const TiXmlDocument * pXmlDoc, cGUIPage * * ppPage);

   void Activate();
   void Deactivate();

   void SetOverlay(bool bIsOverlay) { m_bIsOverlay = bIsOverlay; }
   bool IsOverlay() const { return m_bIsOverlay; }

   bool IsModalDialogPage() const;

   void Clear();

   size_t CountElements() const;
   tResult GetElement(const tChar * pszId, IGUIElement * * ppElement);

   tResult GetActiveModalDialog(IGUIDialogElement * * ppDialog);

   void UpdateLayout(const tGUIRect & rect);
   void Render(IGUIRenderDevice * pRenderDevice);

   tResult GetHitElements(const tGUIPoint & point, tGUIElementList * pElements) const;

private:
   void RunScripts();

   tGUIElementList::const_iterator BeginElements() const { return m_elements.begin(); }
   tGUIElementList::const_iterator EndElements() const { return m_elements.end(); }

   tGUIElementList m_elements;
   bool m_bUpdateLayout;
   bool m_bIsOverlay;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIPAGE_H
