///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guitypes.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIColor
//

const cGUIColor cGUIColor::Black(0,0,0);
const cGUIColor cGUIColor::Red(1,0,0);
const cGUIColor cGUIColor::Green(0,1,0);
const cGUIColor cGUIColor::Yellow(1,1,0);
const cGUIColor cGUIColor::Blue(0,0,1);
const cGUIColor cGUIColor::Magenta(1,0,1);
const cGUIColor cGUIColor::Cyan(0,1,1);
const cGUIColor cGUIColor::DarkGray(0.5,0.5,0.5);
const cGUIColor cGUIColor::Gray(0.75,0.75,0.75);
const cGUIColor cGUIColor::LightGray(0.87f,0.87f,0.87f);
const cGUIColor cGUIColor::White(1,1,1);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontDesc
//

////////////////////////////////////////

cGUIFontDesc::cGUIFontDesc()
 : m_pointSize(0),
   m_effects(0),
   m_glyphFirst(0),
   m_glyphLast(0)
{
}

////////////////////////////////////////

cGUIFontDesc::cGUIFontDesc(const tChar * pszFace, int pointSize, uint effects, uint glyphFirst, uint glyphLast)
 : m_typeFace(pszFace),
   m_pointSize(pointSize),
   m_effects(effects),
   m_glyphFirst(glyphFirst),
   m_glyphLast(glyphLast)
{
}

////////////////////////////////////////

cGUIFontDesc::cGUIFontDesc(const cGUIFontDesc & other)
 : m_typeFace(other.m_typeFace),
   m_pointSize(other.m_pointSize),
   m_effects(other.m_effects),
   m_glyphFirst(other.m_glyphFirst),
   m_glyphLast(other.m_glyphLast)
{
}

////////////////////////////////////////

const cGUIFontDesc & cGUIFontDesc::operator =(const cGUIFontDesc & other)
{
   m_typeFace = other.m_typeFace;
   m_pointSize = other.m_pointSize;
   m_effects = other.m_effects;
   m_glyphFirst = other.m_glyphFirst;
   m_glyphLast = other.m_glyphLast;
   return *this;
}

////////////////////////////////////////

bool cGUIFontDesc::operator ==(const cGUIFontDesc & other) const
{
   return m_typeFace.compare(other.m_typeFace) == 0
      && m_pointSize == other.m_pointSize
      && m_effects == other.m_effects
      && m_glyphFirst == other.m_glyphFirst
      && m_glyphLast == other.m_glyphLast;
}

////////////////////////////////////////

bool cGUIFontDesc::operator <(const cGUIFontDesc & other) const
{
   return m_typeFace.compare(other.m_typeFace) < 0
      || m_pointSize < other.m_pointSize
      || m_effects < other.m_effects
      || m_glyphFirst < other.m_glyphFirst
      || m_glyphLast < other.m_glyphLast;
}

///////////////////////////////////////////////////////////////////////////////
