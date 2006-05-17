///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUISTYLEAPI_H
#define INCLUDED_GUISTYLEAPI_H

/// @file guistyleapi.h
/// Style-related GUI interface definitions

#include "guidll.h"

#include "comtools.h"

#include "guitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGUIStyle);
F_DECLARE_INTERFACE(IGUIStyleSheet);

class cGUIFontDesc;


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIStyle
//
/// @interface IGUIStyle
/// @brief Contains the visual attributes, configured in XML, used to render an element.

interface IGUIStyle : IUnknown
{
   virtual tResult GetAttribute(const tGUIChar * pszAttribute, tGUIString * pValue) = 0;
   virtual tResult GetAttribute(const tGUIChar * pszAttribute, int * pValue) = 0;
   virtual tResult GetAttribute(const tGUIChar * pszAttribute, tGUIColor * pValue) = 0;
   virtual tResult SetAttribute(const tGUIChar * pszAttribute, const tGUIChar * pszValue) = 0;
   virtual tResult SetAttribute(const tGUIChar * pszAttribute, int value) = 0;

   virtual tResult GetAlignment(uint * pAlignment) = 0;
   virtual tResult SetAlignment(uint alignment) = 0;

   virtual tResult GetVerticalAlignment(uint * pAlignment) = 0;
   virtual tResult SetVerticalAlignment(uint alignment) = 0;

   virtual tResult GetBackgroundColor(tGUIColor * pColor) = 0;
   virtual tResult SetBackgroundColor(const tGUIColor & color) = 0;

   virtual tResult GetForegroundColor(tGUIColor * pColor) = 0;
   virtual tResult SetForegroundColor(const tGUIColor & color) = 0;

   virtual tResult GetTextAlignment(uint * pAlignment) = 0;
   virtual tResult SetTextAlignment(uint alignment) = 0;

   virtual tResult GetTextVerticalAlignment(uint * pAlignment) = 0;
   virtual tResult SetTextVerticalAlignment(uint alignment) = 0;

   virtual tResult GetFontName(tGUIString * pFontName) = 0;
   virtual tResult SetFontName(const tGUIChar * pszFontName) = 0;

   virtual tResult GetFontPointSize(uint * pFontPointSize) = 0;
   virtual tResult SetFontPointSize(uint fontPointSize) = 0;

   virtual tResult GetFontBold(bool * pB) = 0;
   virtual tResult SetFontBold(bool b) = 0;

   virtual tResult GetFontItalic(bool * pB) = 0;
   virtual tResult SetFontItalic(bool b) = 0;

   virtual tResult GetFontShadow(bool * pB) = 0;
   virtual tResult SetFontShadow(bool b) = 0;

   virtual tResult GetFontOutline(bool * pB) = 0;
   virtual tResult SetFontOutline(bool b) = 0;

   virtual tResult GetPlacement(uint * pPlacement) const = 0;
   virtual tResult SetPlacement(uint placement) = 0;

   virtual tResult GetWidth(int * pWidth, uint * pSpec) = 0;
   virtual tResult SetWidth(int width, uint spec) = 0;

   virtual tResult GetHeight(int * pHeight, uint * pSpec) = 0;
   virtual tResult SetHeight(int height, uint spec) = 0;

   virtual tResult Clone(IGUIStyle * * ppStyle) = 0;
};

GUI_API tResult GUIStyleFontDesc(IGUIStyle * pStyle, cGUIFontDesc * pFontDesc);

///////////////////////////////////////
/// Attempt to parse a color value from the given string. Valid strings are
/// of the form "(R,G,B,A)". The alpha component is optional. Color components 
/// can be in the range [0,255] or [0,1]. Certain standard colors are 
/// supported, too. For example, "red", "green", "blue", etc.

GUI_API tResult GUIParseColor(const tGUIChar * pszColor, tGUIColor * pColor);

///////////////////////////////////////
/// Parse a CSS-like string to produce a GUI style object.

GUI_API tResult GUIStyleParse(const tGUIChar * pszStyle, long length, IGUIStyle * * ppStyle);

GUI_API tResult GUIStyleParseInline(const tGUIChar * pszStyle, long length,
                                    IGUIStyle * pClassStyle, IGUIStyle * * ppStyle);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIStyleSheet
//

interface IGUIStyleSheet : IUnknown
{
   virtual tResult AddRule(const tGUIChar * pszSelector, IGUIStyle * pStyle) = 0;

   virtual tResult GetStyle(const tGUIChar * pszType, const tGUIChar * pszClass,
                            IGUIStyle * * ppStyle) const = 0;
};

///////////////////////////////////////

GUI_API tResult GUIStyleSheetParse(const tGUIChar * pszStyleSheet,
                                   IGUIStyleSheet * * ppStyleSheet);


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUISTYLEAPI_H
