///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUISTYLE_H
#define INCLUDED_GUISTYLE_H

#include "guistyleapi.h"

#include "dictionaryapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyle
//

class cGUIStyle : public cComObject<IMPLEMENTS(IGUIStyle)>
{
   void operator =(const cGUIStyle &);

   cGUIStyle(IGUIStyle * pClassStyle, IDictionary * pDict = NULL);
   cGUIStyle(const cGUIStyle & other);
   ~cGUIStyle();

public:
   static tResult Create(IGUIStyle * pClassStyle, IGUIStyle * * ppStyle);

   virtual tResult GetAttribute(const tChar * pszAttribute, tGUIString * pValue);
   virtual tResult GetAttribute(const tChar * pszAttribute, int * pValue);
   virtual tResult GetAttribute(const tChar * pszAttribute, tGUIColor * pValue);
   virtual tResult SetAttribute(const tChar * pszAttribute, const tChar * pszValue);
   virtual tResult SetAttribute(const tChar * pszAttribute, int value);

   virtual tResult GetAlignment(uint * pAlignment);
   virtual tResult SetAlignment(uint alignment);

   virtual tResult GetVerticalAlignment(uint * pVerticalAlignment);
   virtual tResult SetVerticalAlignment(uint verticalAlignment);

   virtual tResult GetBackgroundColor(tGUIColor * pColor);
   virtual tResult SetBackgroundColor(const tGUIColor & color);

   virtual tResult GetForegroundColor(tGUIColor * pColor);
   virtual tResult SetForegroundColor(const tGUIColor & color);

   virtual tResult GetTextAlignment(uint * pAlignment);
   virtual tResult SetTextAlignment(uint alignment);

   virtual tResult GetTextVerticalAlignment(uint * pAlignment);
   virtual tResult SetTextVerticalAlignment(uint alignment);

   virtual tResult GetFontName(tGUIString * pFontName);
   virtual tResult SetFontName(const char * pszFontName);

   virtual tResult GetFontPointSize(uint * pFontPointSize);
   virtual tResult SetFontPointSize(uint fontPointSize);

   virtual tResult GetFontBold(bool * pB);
   virtual tResult SetFontBold(bool b);

   virtual tResult GetFontItalic(bool * pB);
   virtual tResult SetFontItalic(bool b);

   virtual tResult GetFontShadow(bool * pB);
   virtual tResult SetFontShadow(bool b);

   virtual tResult GetFontOutline(bool * pB);
   virtual tResult SetFontOutline(bool b);

   virtual tResult GetFontDesc(cGUIFontDesc * pFontDesc);

   virtual tResult GetWidth(int * pWidth, uint * pSpec);
   virtual tResult SetWidth(int width, uint spec);

   virtual tResult GetHeight(int * pHeight, uint * pSpec);
   virtual tResult SetHeight(int height, uint spec);

   virtual tResult Clone(IGUIStyle * * ppStyle);

private:
   cAutoIPtr<IGUIStyle> m_pClassStyle;
   cAutoIPtr<IDictionary> m_pDict;
   uint m_alignment, m_verticalAlignment;
   tGUIColor * m_pBackground, * m_pForeground;
   uint m_textAlignment, m_textVerticalAlignment;
   tGUIString m_fontName;
   uint m_fontPointSize;
   bool m_bFontBold, m_bFontItalic, m_bFontShadow, m_bFontOutline;
   int m_width, m_height;
   uint m_widthSpec, m_heightSpec;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUISTYLE_H
