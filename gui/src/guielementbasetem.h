///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIELEMENTBASETEM_H
#define INCLUDED_GUIELEMENTBASETEM_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIElementBase
//

///////////////////////////////////////

template <typename INTRFC>
cGUIElementBase<INTRFC>::cGUIElementBase()
 : m_id(""),
   m_bFocus(false),
   m_bVisible(true),
   m_bEnabled(true),
   m_position(0,0),
   m_size(0,0)
{
}

///////////////////////////////////////

template <typename INTRFC>
const char * cGUIElementBase<INTRFC>::GetId() const
{
   return m_id;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetId(const char * pszId)
{
   m_id = pszId;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::HasFocus() const
{
   return m_bFocus;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetFocus(bool bFocus)
{
   m_bFocus = bFocus;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::IsVisible() const
{
   return m_bVisible;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetVisible(bool bVisible)
{
   m_bVisible = bVisible;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::IsEnabled() const
{
   return m_bEnabled;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetEnabled(bool bEnabled)
{
   m_bEnabled = bEnabled;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetParent(IGUIElement * * ppParent)
{
   return m_pParent.GetPointer(ppParent);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::SetParent(IGUIElement * pParent)
{
   SafeRelease(m_pParent);
   m_pParent = CTAddRef(pParent);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tGUIPoint cGUIElementBase<INTRFC>::GetPosition() const
{
   return m_position;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetPosition(const tGUIPoint & point)
{
   m_position = point;
}

///////////////////////////////////////

template <typename INTRFC>
tGUISize cGUIElementBase<INTRFC>::GetSize() const
{
   return m_size;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIElementBase<INTRFC>::SetSize(const tGUISize & size)
{
   m_size = size;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIElementBase<INTRFC>::Contains(const tGUIPoint & point)
{
   return false; // TODO
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::OnEvent(IGUIEvent * pEvent)
{
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::GetRenderer(IGUIElementRenderer * * ppRenderer)
{
   return m_pRenderer.GetPointer(ppRenderer);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIElementBase<INTRFC>::SetRenderer(IGUIElementRenderer * pRenderer)
{
   SafeRelease(m_pRenderer);
   m_pRenderer = CTAddRef(pRenderer);
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIELEMENTBASETEM_H
