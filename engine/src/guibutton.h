///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBUTTON_H
#define INCLUDED_GUIBUTTON_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElement
//

class cGUIButtonElement : public cComObject<IMPLEMENTS(IGUIButtonElement)>
{
public:
   cGUIButtonElement();
   ~cGUIButtonElement();

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

   virtual bool IsArmed() const;
   virtual void SetArmed(bool bArmed);

   virtual bool IsMouseOver() const;
   virtual void SetMouseOver(bool bMouseOver);

   virtual const char * GetText() const;
   virtual void SetText(const char * pszText);

private:
   tGUIString m_id;
   bool m_bHasFocus;
   bool m_bVisible;
   bool m_bEnabled;
   cAutoIPtr<IGUIElement> m_pParent;
   tGUIPoint m_position;
   tGUISize m_size;
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

#endif // !INCLUDED_GUIBUTTON_H
