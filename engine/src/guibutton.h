///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBUTTON_H
#define INCLUDED_GUIBUTTON_H

#include "guielementbase.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);

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

   virtual bool IsMouseOver() const;
   virtual void SetMouseOver(bool bMouseOver);

   virtual const char * GetText() const;
   virtual void SetText(const char * pszText);

private:
   bool m_bArmed;
   bool m_bMouseOver;
   tGUIString m_text;
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

private:
   cAutoIPtr<IRenderFont> m_pFont;
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
