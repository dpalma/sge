///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUITEXTEDIT_H
#define INCLUDED_GUITEXTEDIT_H

#include "guielementbase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextBuffer
//

class cGUITextBuffer
{
public:
   cGUITextBuffer();
   ~cGUITextBuffer();

   tResult GetText(tGUIString * pText);
   tResult SetText(const char * pszText);

   void SetCursorIndex(uint index);
   int GetCursorIndex() const;

   void CharRight(int count = 1);
   void CharLeft(int count = 1);

   void WordRight();
   void WordLeft();

   void Start();
   void End();

   void Backspace(int count = 1);
   void Delete(int count = 1);

   void ReplaceSel(char c);
   void ReplaceSel(const char * psz);

   void Cut();
   void Copy();
   void Paste();

private:
   tGUIString::iterator GetCursor();
   tGUIString::const_iterator GetCursor() const;

   ////////////////////////////////////

   tGUIString m_text;
   tGUIString::iterator m_cursor;
};

///////////////////////////////////////

inline tGUIString::iterator cGUITextBuffer::GetCursor()
{
   return m_cursor;
}

///////////////////////////////////////

inline tGUIString::const_iterator cGUITextBuffer::GetCursor() const
{
   return m_cursor;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextEditElement
//

class cGUITextEditElement : public cComObject<cGUIElementBase<IGUITextEditElement>, &IID_IGUITextEditElement>
{
public:
   cGUITextEditElement();
   ~cGUITextEditElement();

   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual tResult GetEditSize(uint * pEditSize);
   virtual tResult SetEditSize(uint editSize);

   virtual tResult GetSelection(uint * pStart, uint * pEnd);
   virtual tResult SetSelection(uint start, uint end);

   virtual tResult GetText(tGUIString * pText);
   virtual tResult SetText(const char * pszText);

   virtual void UpdateBlinkingCursor();
   virtual bool ShowBlinkingCursor() const;

private:
   tResult HandleKeyDown(long keyCode);

   bool HitTest(const tGUIPoint & point, int * pIndex);

   uint m_editSize;

   cGUITextBuffer m_buffer;

   double m_timeLastBlink;
   bool m_bCursorBlinkOn, m_bCursorForceOn;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextEditElementFactory
//

class cGUITextEditElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUITEXTEDIT_H
