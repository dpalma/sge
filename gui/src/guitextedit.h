///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUITEXTEDIT_H
#define INCLUDED_GUITEXTEDIT_H

#include "guielementbase.h"

#include "inputapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextSelection
//

class cGUITextSelection
{
public:
   cGUITextSelection(tGUIString * pText);
   ~cGUITextSelection();

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
   tGUIString * GetText();
   const tGUIString * GetText() const;

   tGUIString::iterator GetCursor();
   tGUIString::const_iterator GetCursor() const;

   ////////////////////////////////////

   tGUIString * m_pText;
   tGUIString::iterator m_cursor;
};

///////////////////////////////////////

inline tGUIString * cGUITextSelection::GetText()
{
   Assert(m_pText != NULL);
   return m_pText;
}

///////////////////////////////////////

inline const tGUIString * cGUITextSelection::GetText() const
{
   Assert(m_pText != NULL);
   return m_pText;
}

///////////////////////////////////////

inline tGUIString::iterator cGUITextSelection::GetCursor()
{
   return m_cursor;
}

///////////////////////////////////////

inline tGUIString::const_iterator cGUITextSelection::GetCursor() const
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

   virtual tResult GetRendererClass(tGUIString * pRendererClass);

   virtual tResult GetEditSize(uint * pEditSize);
   virtual tResult SetEditSize(uint editSize);

   virtual tResult GetSelection(uint * pStart, uint * pEnd);
   virtual tResult SetSelection(uint start, uint end);

   virtual const char * GetText() const;
   virtual void SetText(const char * pszText);

private:
   tResult HandleKeyDown(long keyCode);

   bool HitTest(const tGUIPoint & point, int * pIndex);

   uint m_editSize;

   tGUIString m_text;
   cGUITextSelection m_selection;
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
//
// CLASS: cGUITextEditStatelessRenderer
//

class cGUITextEditStatelessRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUITextEditStatelessRenderer();
   ~cGUITextEditStatelessRenderer();

   virtual tResult Render(IGUIElement * pElement, IRenderDevice * pRenderDevice);

   virtual tGUISize GetPreferredSize(IGUIElement * pElement);

private:
   tResult GetFont(IGUITextEditElement * pTextEditElement, IRenderFont * * ppFont);

   class cInputListener : public cComObject<IMPLEMENTS(IInputListener)>
   {
      friend class cGUITextEditStatelessRenderer;
      cGUITextEditStatelessRenderer * m_pOuter;
      cInputListener(cGUITextEditStatelessRenderer * pOuter);
      virtual bool OnInputEvent(const sInputEvent * pEvent);
   };

   friend class cInputListener;
   cInputListener m_inputListener;

   double m_timeLastBlink;
   bool m_bCursorBlinkOn, m_bCursorForceOn;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextEditStatelessRendererFactory
//

class cGUITextEditStatelessRendererFactory : public cComObject<IMPLEMENTS(IGUIElementRendererFactory)>
{
public:
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer);

private:
   cAutoIPtr<IGUIElementRenderer> m_pStatelessLabelRenderer;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUITEXTEDIT_H
