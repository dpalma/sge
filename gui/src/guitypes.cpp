///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guitypes.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

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
 : m_pointSize(0)
 , m_effects(0)
 , m_glyphFirst(0)
 , m_glyphLast(0)
{
}

////////////////////////////////////////

cGUIFontDesc::cGUIFontDesc(const tChar * pszFace, int pointSize, uint effects, uint glyphFirst, uint glyphLast)
 : m_typeFace(pszFace)
 , m_pointSize(pointSize)
 , m_effects(effects)
 , m_glyphFirst(glyphFirst)
 , m_glyphLast(glyphLast)
{
}

////////////////////////////////////////

cGUIFontDesc::cGUIFontDesc(const cGUIFontDesc & other)
 : m_typeFace(other.m_typeFace)
 , m_pointSize(other.m_pointSize)
 , m_effects(other.m_effects)
 , m_glyphFirst(other.m_glyphFirst)
 , m_glyphLast(other.m_glyphLast)
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
   int result = m_typeFace.compare(other.m_typeFace);

   if (result > 0)
   {
      return false;
   }
   else if (result < 0)
   {
      return true;
   }

   if (m_pointSize > other.m_pointSize)
   {
      return false;
   }
   else if (m_pointSize < other.m_pointSize)
   {
      return true;
   }

   if (m_effects > other.m_effects)
   {
      return false;
   }
   else if (m_effects < other.m_effects)
   {
      return true;
   }

   if (m_glyphFirst > other.m_glyphFirst)
   {
      return false;
   }
   else if (m_glyphFirst < other.m_glyphFirst)
   {
      return true;
   }

   if (m_glyphLast > other.m_glyphLast)
   {
      return false;
   }
   else if (m_glyphLast < other.m_glyphLast)
   {
      return true;
   }

   return false;
}

////////////////////////////////////////

const tGUIChar * cGUIFontDesc::GetFace() const
{
   return m_typeFace.c_str();
}

////////////////////////////////////////

int cGUIFontDesc::GetPointSize() const
{
   return m_pointSize;
}

////////////////////////////////////////

bool cGUIFontDesc::IsBold() const
{
   return (m_effects & kGFE_Bold) == kGFE_Bold;
}

////////////////////////////////////////

bool cGUIFontDesc::IsItalic() const
{
   return (m_effects & kGFE_Italic) == kGFE_Italic;
}

////////////////////////////////////////

bool cGUIFontDesc::IsShadow() const
{
   return (m_effects & kGFE_Shadow) == kGFE_Shadow;
}

////////////////////////////////////////

bool cGUIFontDesc::IsOutline() const
{
   return (m_effects & kGFE_Outline) == kGFE_Outline;
}

////////////////////////////////////////

uint cGUIFontDesc::GetGlyphFirst() const
{
   return m_glyphFirst;
}

////////////////////////////////////////

uint cGUIFontDesc::GetGlyphLast() const
{
   return m_glyphLast;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

TEST(GUIFontDescLessThan)
{
   {
      // 1 < 2 because of name
      cGUIFontDesc fontDesc1(_T("Arial"), 12, kGFE_None);
      cGUIFontDesc fontDesc2(_T("Times"), 12, kGFE_None);
      CHECK(fontDesc1 < fontDesc2);
      CHECK(!(fontDesc2 < fontDesc1));
   }

   {
      // 1 < 2 because of point size
      cGUIFontDesc fontDesc1(_T("Arial"), 12, kGFE_None);
      cGUIFontDesc fontDesc2(_T("Arial"), 14, kGFE_None);
      CHECK(fontDesc1 < fontDesc2);
      CHECK(!(fontDesc2 < fontDesc1));
   }

   {
      // 1 < 2 because of font effects
      cGUIFontDesc fontDesc1(_T("Arial"), 12, kGFE_None);
      cGUIFontDesc fontDesc2(_T("Arial"), 12, kGFE_Bold);
      CHECK(fontDesc1 < fontDesc2);
      CHECK(!(fontDesc2 < fontDesc1));
   }

   {
      // 1 < 2 because of m_glyphFirst
      cGUIFontDesc fontDesc1(_T("Arial"), 12, kGFE_Bold, cGUIFontDesc::kANSIGlyphFirst - 1);
      cGUIFontDesc fontDesc2(_T("Arial"), 12, kGFE_Bold, cGUIFontDesc::kANSIGlyphFirst);
      CHECK(fontDesc1 < fontDesc2);
      CHECK(!(fontDesc2 < fontDesc1));
   }

   {
      // 1 < 2 because of m_glyphLast
      cGUIFontDesc fontDesc1(_T("Arial"), 12, kGFE_Bold, cGUIFontDesc::kANSIGlyphFirst, cGUIFontDesc::kANSIGlyphLast - 1);
      cGUIFontDesc fontDesc2(_T("Arial"), 12, kGFE_Bold, cGUIFontDesc::kANSIGlyphFirst, cGUIFontDesc::kANSIGlyphLast);
      CHECK(fontDesc1 < fontDesc2);
      CHECK(!(fontDesc2 < fontDesc1));
   }

   {
      // 1 == 2
      cGUIFontDesc fontDesc1(_T("Arial"), 12, kGFE_Bold);
      cGUIFontDesc fontDesc2(_T("Arial"), 12, kGFE_Bold);
      CHECK(!(fontDesc1 < fontDesc2));
      CHECK(!(fontDesc2 < fontDesc1));
   }

   {
      // Cover less-than operator bug discovered 4/25/2006
      static const cGUIFontDesc noName10(_T(""), 10, kGFE_None);
      static const cGUIFontDesc courierNew8(_T("Courier New"), 8, kGFE_None);
      CHECK(noName10 < courierNew8);
      CHECK(!(courierNew8 < noName10));
   }
}

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
