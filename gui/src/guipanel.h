///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIPANEL_H
#define INCLUDED_GUIPANEL_H

#include "guiapi.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPanelElement
//

class cGUIPanelElement : public cComObject<IMPLEMENTS(IGUIPanelElement)>
{
public:
   cGUIPanelElement();
   ~cGUIPanelElement();

   virtual const char * GetId() const;
   virtual void SetId(const char * pszId);

   virtual bool HasFocus() const;
   virtual void SetFocus(bool bFocus);

   virtual bool IsVisible() const;
   virtual void SetVisible(bool bVisible);

   virtual bool IsEnabled() const;
   virtual void SetEnabled(bool bEnabled);

   virtual tResult GetParent(IGUIElement * * ppParent);
   virtual tResult SetParent(IGUIElement * pParent);

   virtual tGUIPoint GetPosition() const;
   virtual void SetPosition(const tGUIPoint & point);

   virtual tGUISize GetSize() const;
   virtual void SetSize(const tGUISize & size);

   virtual bool Contains(const tGUIPoint & point);

   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual tResult GetRendererClass(tGUIString * pRendererClass);
   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer);
   virtual tResult SetRenderer(IGUIElementRenderer * pRenderer);

   virtual tResult AddElement(IGUIElement * pElement);
   virtual tResult RemoveElement(IGUIElement * pElement);
   virtual tResult GetElements(IGUIElementEnum * * ppElements);

private:
   tGUIString m_id;
   bool m_bVisible;
   bool m_bEnabled;
   cAutoIPtr<IGUIElement> m_pParent;
   tGUIPoint m_position;
   tGUISize m_size;
   cAutoIPtr<IGUIElementRenderer> m_pRenderer;
   typedef std::list<IGUIElement *> tGUIElementList;
   tGUIElementList m_children;
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

private:
   cAutoIPtr<IRenderFont> m_pFont;
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
