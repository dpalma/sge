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

   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
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

   virtual tResult GetRendererClass(tGUIString * pRendererClass);

   virtual tResult GetInsets(tGUIInsets * pInsets);
   virtual tResult SetInsets(const tGUIInsets & insets);

private:
   tGUIInsets * m_pInsets;
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
//
// CLASS: cGUIPanelRenderer
//

class cGUIPanelRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUIPanelRenderer();
   ~cGUIPanelRenderer();

   virtual tResult Render(IGUIElement * pElement, IRenderDevice * pRenderDevice);

   virtual tGUISize GetPreferredSize(IGUIElement * pElement);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelRendererFactory
//

class cGUIPanelRendererFactory : public cComObject<IMPLEMENTS(IGUIElementRendererFactory)>
{
public:
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIPANEL_H
