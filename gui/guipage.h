///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIPAGE_H
#define INCLUDED_GUIPAGE_H

#include "guielementbase.h"

#include "tech/connptimpl.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

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

   bool IsModalDialogPage() const;

   tResult GetElement(const tChar * pszId, IGUIElement * * ppElement);

   void RequestLayout(IGUIElement * pRequester, uint options);

   void UpdateLayout(const tGUIRect & rect);
   void Render();

   tResult GetHitElements(const tScreenPoint & point, tGUIElementList * pElements) const;

private:
   void ClearElements();
   void ClearLayoutRequests();

   void RunScripts();

   tGUIElementList::const_iterator BeginElements() const { return m_elements.begin(); }
   tGUIElementList::const_iterator EndElements() const { return m_elements.end(); }

   tGUIElementList m_elements;

   typedef std::list<std::pair<IGUIElement*, uint> > tLayoutRequests;
   tLayoutRequests m_layoutRequests;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIPAGE_H
