///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTBASE_H
#define INCLUDED_GUIELEMENTBASE_H

#include "gui/guielementapi.h"

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
   virtual void SetId(const tChar * pszId);

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

   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer);
   virtual tResult SetRenderer(IGUIElementRenderer * pRenderer);

   virtual tResult GetStyle(IGUIStyle * * ppStyle);
   virtual tResult SetStyle(IGUIStyle * pStyle);

   virtual tResult EnumChildren(IGUIElementEnum * * ppElements);

   virtual tResult GetClientArea(tGUIRect * pClientArea);
   virtual tResult SetClientArea(const tGUIRect & clientArea);

   virtual tResult ComputeClientArea(IGUIElementRenderer * pRenderer, tGUIRect * pClientArea);

private:
   enum eGUIElementInternalFlags
   {
      kFlags_None             = 0,
      kFlags_Visible          = 1 << 1,
      kFlags_Disabled         = 1 << 2,
   };

   tGUIString m_id;
   uint8 m_flags;
   IGUIElement * m_pParent;
   tGUIPoint m_position;
   tGUISize m_size;
   tGUIRect * m_pClientArea;
   cAutoIPtr<IGUIElementRenderer> m_pRenderer;
   cAutoIPtr<IGUIStyle> m_pStyle;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTBASE_H
