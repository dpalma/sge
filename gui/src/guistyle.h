///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUISTYLE_H
#define INCLUDED_GUISTYLE_H

#include "guielementbase.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyle
//

class cGUIStyle : public cComObject<IMPLEMENTS(IGUIStyle)>
{
public:
   cGUIStyle();
   ~cGUIStyle();

   virtual tResult GetAttribute(const char * pszAttribute, tGUIString * pValue);
   virtual tResult GetAttribute(const char * pszAttribute, uint * pValue);
   virtual tResult GetAttribute(const char * pszAttribute, tGUIColor * pValue);
   virtual tResult SetAttribute(const char * pszAttribute, const char * pszValue);

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

   virtual tResult GetWidth(uint * pWidth, uint * pSpec);
   virtual tResult SetWidth(uint width, uint spec);

   virtual tResult GetHeight(uint * pHeight, uint * pSpec);
   virtual tResult SetHeight(uint height, uint spec);

private:
   typedef std::map<cStr, cStr> tAttributeMap;
   tAttributeMap m_attributeMap;
   uint m_alignment, m_verticalAlignment;
   tGUIColor * m_pBackground, * m_pForeground;
   uint m_textAlignment, m_textVerticalAlignment;
   tGUIString m_fontName;
   uint m_fontPointSize;
   bool m_bFontBold, m_bFontItalic, m_bFontShadow, m_bFontOutline;
   uint m_width, m_widthSpec;
   uint m_height, m_heightSpec;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyleElement
//

class cGUIStyleElement : public cComObject<cGUIElementBase<IGUIStyleElement>, &IID_IGUIStyleElement>
{
public:
   cGUIStyleElement();
   ~cGUIStyleElement();

   virtual tResult GetRendererClass(tGUIString * pRendererClass);
   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyleElementFactory
//

class cGUIStyleElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * pParent, IGUIElement * * ppElement);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUISTYLE_H
