///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBUTTON_H
#define INCLUDED_GUIBUTTON_H

#include "guielementbase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElement
//

class cGUIButtonElement : public cComObject<cGUIElementBase<IGUIButtonElement>, &IID_IGUIButtonElement>
{
public:
   cGUIButtonElement();
   ~cGUIButtonElement();

   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual tResult GetRendererClass(tGUIString * pRendererClass);

   virtual bool IsArmed() const;
   virtual void SetArmed(bool bArmed);

   virtual const char * GetText() const;
   virtual void SetText(const char * pszText);

   virtual tResult GetOnClick(tGUIString * pOnClick) const;
   virtual tResult SetOnClick(const char * pszOnClick);

private:
   bool m_bArmed;
   tGUIString m_text;
   tGUIString m_onClick;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElementFactory
//

class cGUIButtonElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement);
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonRenderer
//

class cGUIButtonRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUIButtonRenderer();
   ~cGUIButtonRenderer();

   virtual tResult Render(IGUIElement * pElement, IRenderDevice * pRenderDevice);

   virtual tGUISize GetPreferredSize(IGUIElement * pElement);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonRendererFactory
//

class cGUIButtonRendererFactory : public cComObject<IMPLEMENTS(IGUIElementRendererFactory)>
{
public:
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIBUTTON_H
