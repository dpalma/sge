///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIWIDGUTILS_H
#define INCLUDED_UIWIDGUTILS_H

#include "comtools.h"
#include <map>
#include "uievent.h"
#include "input.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IFont);

F_DECLARE_INTERFACE(IUIStyle);

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cUIDragSemantics
//

template <class T>
class cUIDragSemantics : public cComObject<cDefaultInputListener, &IID_IInputListener>
{
public:
   cUIDragSemantics();
   ~cUIDragSemantics();

   bool FilterEvent(const cUIEvent * pEvent, tUIResult * pResult);

   bool QueryStartDrag(const cUIPoint & mousePos);

   void UpdateDrag(const cUIPoint & mousePos);

   void EndDrag(const cUIPoint & mousePos);

   bool IsDragging() const;

   virtual bool OnKeyEvent(long key, bool down, double time);

private:
   bool m_bDragging;
};


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cUIClickSemantics
//

template <class T>
class cUIClickSemantics : public cUIDragSemantics<T>
{
public:
   void EndDrag(const cUIPoint & mousePos);

protected:
   bool IsMouseOver() const;
   bool IsPressed();
};


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cUIScriptEventHandler
//

template <class T>
class cUIScriptEventHandler
{
public:
   void SetEventHandler(eUIEventCode code, const char * pszScriptCode);

protected:
   bool FilterEvent(const cUIEvent * pEvent, tUIResult * pResult);

private:
   bool CallEventHandler(eUIEventCode code);

   ////////////////////////////////////

   typedef std::map<eUIEventCode, cUIString> tEventHandlerMap;
   tEventHandlerMap m_scriptEventHandlers;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUITextSelection
//

class cUITextSelection
{
public:
   cUITextSelection(cUIString * pText);

   void SetCursorFromPoint(const cUIPoint & point);
   int GetCursorIndex() const;

   ////////////////////////////////////

   void CharRight(int count = 1);
   void CharLeft(int count = 1);

   void WordRight();
   void WordLeft();

   void Start();
   void End();

   ////////////////////////////////////

   void Backspace(int count = 1);
   void Delete(int count = 1);

   void ReplaceSel(char c);
   void ReplaceSel(const char * psz);

   void Cut();
   void Copy();
   void Paste();

private:
   ////////////////////////////////////

   bool HitTest(const cUIPoint & point, cUIString::iterator * pIter);

   ////////////////////////////////////

   cUIString * GetText();
   const cUIString * GetText() const;

   cUIString::iterator GetCursor();
   cUIString::const_iterator GetCursor() const;

   ////////////////////////////////////

   cUIString * m_pText;
   cUIString::iterator m_cursor;
};

///////////////////////////////////////

inline cUIString * cUITextSelection::GetText()
{
   Assert(m_pText != NULL);
   return m_pText;
}

///////////////////////////////////////

inline const cUIString * cUITextSelection::GetText() const
{
   Assert(m_pText != NULL);
   return m_pText;
}

///////////////////////////////////////

inline cUIString::iterator cUITextSelection::GetCursor()
{
   return m_cursor;
}

///////////////////////////////////////

inline cUIString::const_iterator cUITextSelection::GetCursor() const
{
   return m_cursor;
}

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IUIStyle
//

interface IUIStyle : IUnknown
{
   virtual cUIColor GetHighlight() const = 0;
   virtual cUIColor GetShadow() const = 0;
   virtual cUIColor GetFace() const = 0;
   virtual cUIColor GetText() const = 0;
   virtual cUIColor GetHot() const = 0;
   virtual cUIColor GetCaption() const = 0;
   virtual cUIColor GetCaptionText() const = 0;
   virtual cUIColor GetCaptionGradient() const = 0;
   virtual cUIColor GetBackground() const = 0;

   virtual IFont * AccessFont() const = 0;
};

IUIStyle * UIStyleCreateDefault();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_UIWIDGUTILS_H
