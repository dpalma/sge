///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTBASE_H
#define INCLUDED_GUIELEMENTBASE_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIElementBase
//

template <typename INTRFC>
class cGUIElementBase : public INTRFC
{
public:
   cGUIElementBase();

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

   virtual bool Contains(const tGUIPoint & point) const;

   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer);
   virtual tResult SetRenderer(IGUIElementRenderer * pRenderer);

   virtual tResult GetStyle(IGUIStyle * * ppStyle);
   virtual tResult SetStyle(IGUIStyle * pStyle);

protected:
   tGUIPoint GetAbsolutePosition() const;

private:
   tGUIString m_id;
   bool m_bFocus;
   bool m_bVisible;
   bool m_bEnabled;
   cAutoIPtr<IGUIElement> m_pParent;
   tGUIPoint m_position;
   tGUISize m_size;
   cAutoIPtr<IGUIElementRenderer> m_pRenderer;
   cAutoIPtr<IGUIStyle> m_pStyle;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTBASE_H
