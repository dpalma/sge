///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guitypes.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

namespace GUIStandardColors
{
   const tGUIColor Black(0,0,0);
   const tGUIColor Red(1,0,0);
   const tGUIColor Green(0,1,0);
   const tGUIColor Yellow(1,1,0);
   const tGUIColor Blue(0,0,1);
   const tGUIColor Magenta(1,0,1);
   const tGUIColor Cyan(0,1,1);
   const tGUIColor DarkGray(0.5,0.5,0.5);
   const tGUIColor Gray(0.75,0.75,0.75);
   const tGUIColor LightGray(0.87f,0.87f,0.87f);
   const tGUIColor White(1,1,1);
}


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
