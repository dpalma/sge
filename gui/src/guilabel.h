///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUILABEL_H
#define INCLUDED_GUILABEL_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElement
//

class cGUILabelElement : public cComObject<IMPLEMENTS(IGUILabelElement)>
{
public:
   cGUILabelElement();
   ~cGUILabelElement();

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

   virtual const char * GetText() const;
   virtual void SetText(const char * pszText);

private:
   tGUIString m_id;
   bool m_bVisible;
   bool m_bEnabled;
   cAutoIPtr<IGUIElement> m_pParent;
   tGUIPoint m_position;
   tGUISize m_size;
   cAutoIPtr<IGUIElementRenderer> m_pRenderer;
   tGUIString m_text;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElementFactory
//

class cGUILabelElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelRenderer
//

class cGUILabelRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUILabelRenderer();
   ~cGUILabelRenderer();

   virtual tResult Render(IGUIElement * pElement, IRenderDevice * pRenderDevice);

   virtual tGUISize GetPreferredSize(IGUIElement * pElement);

private:
   cAutoIPtr<IRenderFont> m_pFont;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelRendererFactory
//

class cGUILabelRendererFactory : public cComObject<IMPLEMENTS(IGUIElementRendererFactory)>
{
public:
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUILABEL_H
