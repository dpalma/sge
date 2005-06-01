///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIPANEL_H
#define INCLUDED_GUIPANEL_H

#include "guielementbase.h"
#include "guicontainerbase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelElement
//

class cGUIPanelElement : public cComObject<cGUIElementBase< cGUIContainerBase<IGUIPanelElement> >, 
                                           &IID_IGUIPanelElement>
{
   typedef cComObject<cGUIElementBase< cGUIContainerBase<IGUIPanelElement> >, 
                      &IID_IGUIPanelElement> tBaseClass;
public:
   cGUIPanelElement();
   ~cGUIPanelElement();

   virtual tResult STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                                    void * * ppvObject)
   {
      const struct sQIPair pairs[] =
      {
         { static_cast<IGUIPanelElement *>(this), &IID_IGUIPanelElement },
         { static_cast<IGUIContainerElement *>(this), &IID_IGUIContainerElement }
      };
      return DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }

   virtual void SetSize(const tGUISize & size);
   virtual tResult OnEvent(IGUIEvent * pEvent);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelElementFactory
//

class cGUIPanelElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIPANEL_H
