///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUISTYLE_H
#define INCLUDED_GUISTYLE_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyle
//

class cGUIStyle : public cComObject<IMPLEMENTS(IGUIStyle)>
{
public:
   cGUIStyle();
   ~cGUIStyle();

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

   virtual tResult GetFont(IRenderFont * * ppFont);

   virtual tResult GetWidth(uint * pWidth, uint * pSpec);
   virtual tResult SetWidth(uint width, uint spec);

   virtual tResult GetHeight(uint * pHeight, uint * pSpec);
   virtual tResult SetHeight(uint height, uint spec);

private:
   uint m_alignment, m_verticalAlignment;
   tGUIColor * m_pBackground, * m_pForeground;
   uint m_textAlignment, m_textVerticalAlignment;
   tGUIString m_fontName;
   uint m_fontPointSize;
   cAutoIPtr<IRenderFont> m_pFont;
   uint m_width, m_widthSpec;
   uint m_height, m_heightSpec;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUISTYLE_H
