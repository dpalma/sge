///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIPAGE_H
#define INCLUDED_GUIPAGE_H

#include "guieventrouter.h"

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

class cGUIPage : public cGUIEventRouter<cGUIPage>
{
   cGUIPage(const tGUIElementList * pElements, cGUINotifyListeners * pNotifyListeners);

public:
   ~cGUIPage();

   static tResult Create(const TiXmlDocument * pXmlDoc, cGUINotifyListeners * pNotifyListeners, cGUIPage * * ppPage);

   void Activate();
   void Deactivate();

   bool IsModalDialogPage() const;

   tResult GetElement(const tChar * pszId, IGUIElement * * ppElement);

   void RequestLayout(IGUIElement * pRequester, uint options);

   void UpdateLayout(const tGUIRect & rect);

   tResult RenderElement(IGUIElement * pElement, IGUIElementRenderer * pRenderer, const tGUIPoint & position);

   void Render();

   bool NotifyListeners(IGUIEvent * pEvent);

   tResult GetHitElement(const tScreenPoint & point, IGUIElement * * ppElement) const;
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

   cGUINotifyListeners * m_pNotifyListeners;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIPAGE_H
