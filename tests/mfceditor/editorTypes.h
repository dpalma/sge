/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORTYPES_H)
#define INCLUDED_EDITORTYPES_H

#include "editorapi.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef CPoint tPoint;


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorKeyEvent
//

class cEditorKeyEvent
{
   cEditorKeyEvent(const cEditorKeyEvent &);
   void operator =(const cEditorKeyEvent &);

public:
   cEditorKeyEvent(WPARAM wParam, LPARAM lParam);

   uint GetChar() const { return m_char; }
   uint GetRepeatCount() const { return m_repeats; }
   uint GetFlags() const { return m_flags; }

private:
   uint m_char, m_repeats, m_flags;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseEvent
//

class cEditorMouseEvent
{
   cEditorMouseEvent(const cEditorMouseEvent &);
   void operator =(const cEditorMouseEvent &);

public:
   cEditorMouseEvent(const CPoint & point);
   cEditorMouseEvent(WPARAM wParam, LPARAM lParam);

   uint GetFlags() const { return m_flags; }
   CPoint GetPoint() const { return m_point; }

private:
   uint m_flags;
   CPoint m_point;
};


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorMouseWheelEvent
//

class cEditorMouseWheelEvent : public cEditorMouseEvent
{
   cEditorMouseWheelEvent(const cEditorMouseWheelEvent &);
   void operator =(const cEditorMouseWheelEvent &);

public:
   cEditorMouseWheelEvent(WPARAM wParam, LPARAM lParam);

   short GetZDelta() const { return m_zDelta; }

private:
   short m_zDelta;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_EDITORTYPES_H)
