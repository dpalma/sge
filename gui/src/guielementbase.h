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
   virtual ~cGUIElementBase();

   virtual tResult GetId(tGUIString * pId) const;
   virtual void SetId(const char * pszId);

   virtual bool HasFocus() const;
   virtual void SetFocus(bool bFocus);

   virtual bool IsMouseOver() const;
   virtual void SetMouseOver(bool bMouseOver);

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

   virtual tResult GetRendererClass(tGUIString * pRendererClass);
   virtual tResult SetRendererClass(const tChar * pszRendererClass);

   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer);
   virtual tResult SetRenderer(IGUIElementRenderer * pRenderer);

   virtual tResult GetStyle(IGUIStyle * * ppStyle);
   virtual tResult SetStyle(IGUIStyle * pStyle);

   virtual tResult EnumChildren(IGUIElementEnum * * ppElements);

   virtual tResult GetClientArea(tGUIRect * pClientArea);
   virtual tResult SetClientArea(const tGUIRect & clientArea);

protected:
   tGUIPoint GetAbsolutePosition() const;

private:
   enum eGUIElementInternalFlags
   {
      kFlags_None             = 0,
      kFlags_Focus            = 1 << 0,
      kFlags_MouseOver        = 1 << 1,
      kFlags_Visible          = 1 << 2,
      kFlags_Disabled         = 1 << 3,
      kFlags_HasClientArea    = 1 << 4,
   };

   tGUIString m_id;
   uint m_flags;
   IGUIElement * m_pParent;
   tGUIPoint m_position;
   tGUISize m_size;
   tGUIRect m_clientArea;
   tGUIString m_rendererClass;
   cAutoIPtr<IGUIElementRenderer> m_pRenderer;
   cAutoIPtr<IGUIStyle> m_pStyle;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTBASE_H
