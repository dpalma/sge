///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ui.h"
#include "uiwidgutils.h"
#include "uirender.h"
#include "font.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

static const char kWhiteSpaceChars[] = " \n\r\t";

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUITextSelection
//

cUITextSelection::cUITextSelection(cUIString * pText)
 : m_pText(pText)
{
   End();
}

///////////////////////////////////////

void cUITextSelection::SetCursorFromPoint(const cUIPoint & point)
{
   HitTest(point, &m_cursor);
}

///////////////////////////////////////

int cUITextSelection::GetCursorIndex() const
{
   return GetCursor() - GetText()->begin();
}

///////////////////////////////////////

void cUITextSelection::CharRight(int count)
{
   while (count-- && m_cursor != GetText()->end())
      m_cursor++;
}

///////////////////////////////////////

void cUITextSelection::CharLeft(int count)
{
   while (count-- && m_cursor != GetText()->begin())
      m_cursor--;
}

///////////////////////////////////////

void cUITextSelection::WordRight()
{
   cUIString::size_type start = GetCursor() - GetText()->begin();

   cUIString::size_type index = GetText()->find_first_of(kWhiteSpaceChars, start);

   if (index != std::string::npos)
   {
      index = GetText()->find_first_not_of(kWhiteSpaceChars, index);

      if (index != std::string::npos)
      {
         m_cursor = GetText()->begin() + index;
      }
      else
      {
         m_cursor = GetText()->end();
      }
   }
   else
   {
      m_cursor = GetText()->end();
   }
}

///////////////////////////////////////

void cUITextSelection::WordLeft()
{
   cUIString::size_type index = GetText()->find_last_not_of(kWhiteSpaceChars, GetCursorIndex());

   if (index > 0 && index == GetCursorIndex())
   {
      index = GetText()->find_last_of(kWhiteSpaceChars, index - 1);

      if (index > 0 && index != std::string::npos)
      {
         index = GetText()->find_last_not_of(kWhiteSpaceChars, index - 1);
      }
   }

   if (index != std::string::npos)
   {
      index = GetText()->find_last_of(kWhiteSpaceChars, index);

      if (index != std::string::npos)
      {
         m_cursor = GetText()->begin() + index + 1;
      }
      else
      {
         m_cursor = GetText()->begin();
      }
   }
}

///////////////////////////////////////

void cUITextSelection::Start()
{
   m_cursor = GetText()->begin();
}

///////////////////////////////////////

void cUITextSelection::End()
{
   m_cursor = GetText()->end();
}

///////////////////////////////////////

void cUITextSelection::Backspace(int count)
{
   while (count-- && m_cursor != GetText()->begin())
      m_cursor = GetText()->erase(--m_cursor);
}

///////////////////////////////////////

void cUITextSelection::Delete(int count)
{
   GetText()->erase(GetCursor() - GetText()->begin(), count);
}

///////////////////////////////////////

void cUITextSelection::ReplaceSel(char c)
{
   if (m_cursor == GetText()->end())
   {
      GetText()->append(1, c);
      m_cursor = GetText()->end();
   }
   else
   {
      m_cursor = GetText()->insert(m_cursor, c);
      m_cursor++;
   }
}

///////////////////////////////////////

void cUITextSelection::ReplaceSel(const char * psz)
{
   Assert(psz != NULL);
   if (m_cursor == GetText()->end())
   {
      GetText()->append(psz);
      m_cursor = GetText()->end();
   }
   else
   {
      GetText()->insert(m_cursor - GetText()->begin(), psz);
      m_cursor += strlen(psz);
   }
}

///////////////////////////////////////

void cUITextSelection::Cut()
{
   // @TODO
}

///////////////////////////////////////

void cUITextSelection::Copy()
{
   // @TODO
}

///////////////////////////////////////

void cUITextSelection::Paste()
{
   // @TODO
}

///////////////////////////////////////

bool cUITextSelection::HitTest(const cUIPoint & point, cUIString::iterator * pIter)
{
   float charPos = 0;

   cUIString::iterator iter;
   for (iter = GetText()->begin(); iter != GetText()->end(); iter++)
   {
      cUISize charSize = UIMeasureText(&(*iter), 1);

      if (point.x > charPos && point.x <= charPos + charSize.width)
      {
         if (pIter != NULL)
            *pIter = iter;
         return true;
      }

      charPos += charSize.width;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIStyle
//

class cUIStyle : public cComObject<IMPLEMENTS(IUIStyle)>
{
public:
   cUIStyle();

   cUIColor GetHighlight() const;
   cUIColor GetShadow() const;
   cUIColor GetFace() const;
   cUIColor GetText() const;
   cUIColor GetHot() const;
   cUIColor GetCaption() const;
   cUIColor GetCaptionText() const;
   cUIColor GetCaptionGradient() const;
   cUIColor GetBackground() const;

   IRenderFont * AccessFont() const;

private:
   cAutoIPtr<IRenderFont> m_pFont;
};

///////////////////////////////////////

IUIStyle * UIStyleCreateDefault()
{
   return static_cast<IUIStyle *>(new cUIStyle);
}

///////////////////////////////////////

#ifdef _WIN32

enum eWindowsSysColors
{
   COLOR_ACTIVECAPTION = 2,
   COLOR_WINDOW = 5,
   COLOR_CAPTIONTEXT = 9,
   COLOR_BTNFACE = 15,
   COLOR_BTNSHADOW = 16,
   COLOR_BTNTEXT = 18,
   COLOR_BTNHIGHLIGHT = 20,
   COLOR_GRADIENTACTIVECAPTION = 27,
};

extern "C" __declspec(dllimport) uint32 __stdcall GetSysColor(int);

static cUIColor GetWindowsColor(int index)
{
   uint rgb = GetSysColor(index);
   if (rgb == (uint)-1)
      return cUIColor();
   return cUIColor(
      (float)((byte)rgb) / 255,
      (float)((byte)(((uint16)rgb)>>8)) / 255,
      (float)((byte)(rgb>>16)) / 255,
      1);
}

#endif

///////////////////////////////////////

cUIStyle::cUIStyle()
 : m_pFont(FontCreateDefault())
{
}

///////////////////////////////////////

cUIColor cUIStyle::GetHighlight() const
{
#ifdef _WIN32
   return GetWindowsColor(COLOR_BTNHIGHLIGHT);
#else
   return cUIColor(1,1,1,1);
#endif
}

///////////////////////////////////////

cUIColor cUIStyle::GetShadow() const
{
#ifdef _WIN32
   return GetWindowsColor(COLOR_BTNSHADOW);
#else
   return cUIColor(0.5f,0.5f,0.5f,1);
#endif
}

///////////////////////////////////////

cUIColor cUIStyle::GetFace() const
{
#ifdef _WIN32
   return GetWindowsColor(COLOR_BTNFACE);
#else
   return cUIColor(0.75f,0.75f,0.75f,1);
#endif
}

///////////////////////////////////////

cUIColor cUIStyle::GetText() const
{
#ifdef _WIN32
   return GetWindowsColor(COLOR_BTNTEXT);
#else
   return cUIColor(0,0,0,1);
#endif
}

///////////////////////////////////////

cUIColor cUIStyle::GetHot() const
{
#ifdef _WIN32
   return GetWindowsColor(26);
#else
   return cUIColor(0,0,1,1);
#endif
}

///////////////////////////////////////

cUIColor cUIStyle::GetCaption() const
{
#ifdef _WIN32
   return GetWindowsColor(COLOR_ACTIVECAPTION);
#else
   return cUIColor(0, 0, 0.5f, 1);
#endif
}

///////////////////////////////////////

cUIColor cUIStyle::GetCaptionText() const
{
#ifdef _WIN32
   return GetWindowsColor(COLOR_CAPTIONTEXT);
#else
   return cUIColor(1,1,1,1);
#endif
}

///////////////////////////////////////

cUIColor cUIStyle::GetCaptionGradient() const
{
#ifdef _WIN32
   return GetWindowsColor(COLOR_GRADIENTACTIVECAPTION);
#else
   return cUIColor(0.05f, 0.5f, 0.8f, 1);
#endif
}

///////////////////////////////////////

cUIColor cUIStyle::GetBackground() const
{
#ifdef _WIN32
   return GetWindowsColor(COLOR_WINDOW);
#else
   return cUIColor(1,1,1,1);
#endif
}

///////////////////////////////////////

IRenderFont * cUIStyle::AccessFont() const
{
   return const_cast<cUIStyle *>(this)->m_pFont;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cUITextSelectionTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cUITextSelectionTests);
      CPPUNIT_TEST(TestBackspace);
      CPPUNIT_TEST(TestBackspace2);
      CPPUNIT_TEST(TestBackspace3);
      CPPUNIT_TEST(TestDelete);
      CPPUNIT_TEST(TestDelete2);
      CPPUNIT_TEST(TestWordRight);
      CPPUNIT_TEST(TestWordLeft);
      CPPUNIT_TEST(TestReplace);
   CPPUNIT_TEST_SUITE_END();

public:
   ////////////////////////////////////

   void TestBackspace()
   {
      char szTestString[] = "This is the test string";
      cUIString text(szTestString);
      cUITextSelection sel(&text);

      int i;
      const int nBackspaces = 4;

      sel.Start();
      for (i = 0; i < nBackspaces; i++)
      {
         sel.Backspace();
      }
      CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

      sel.End();
      for (i = 0; i < nBackspaces; i++)
      {
         sel.Backspace();
      }
      szTestString[strlen(szTestString) - nBackspaces] = 0;
      CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);
   }

   ////////////////////////////////////

   void TestBackspace2()
   {
      char szTestString[] = "This is the test string";
      cUIString text(szTestString);
      cUITextSelection sel(&text);

      const int nBackspaces = 5;

      sel.Start();
      sel.Backspace(nBackspaces);
      CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

      sel.End();
      sel.Backspace(nBackspaces);
      szTestString[strlen(szTestString) - nBackspaces] = 0;
      CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);
   }

   ////////////////////////////////////

   void TestBackspace3()
   {
      cUIString text("xxxHELLO");
      cUITextSelection sel(&text);

      sel.Start();
      sel.CharRight(3);
      sel.Backspace(5);
      CPPUNIT_ASSERT(strcmp(text.c_str(), "HELLO") == 0);
   }

   ////////////////////////////////////

   void TestDelete()
   {
      char szTestString[] = "This is the test string";
      cUIString text(szTestString);
      cUITextSelection sel(&text);

      int i;
      const int nDeletes = 4;

      sel.End();
      for (i = 0; i < nDeletes; i++)
      {
         sel.Delete();
      }
      CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

      sel.Start();
      for (i = 0; i < nDeletes; i++)
      {
         sel.Delete();
      }
      CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString + nDeletes) == 0);
   }

   ////////////////////////////////////

   void TestDelete2()
   {
      char szTestString[] = "This is the test string";
      cUIString text(szTestString);
      cUITextSelection sel(&text);

      const int nDeletes = 7;

      sel.End();
      sel.Delete(nDeletes);
      CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

      sel.Start();
      sel.Delete(nDeletes);
      CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString + nDeletes) == 0);
   }

   ////////////////////////////////////

   void TestWordRight()
   {
      char szTestString[] = "This is the test string";
      cUIString text(szTestString);
      cUITextSelection sel(&text);

      sel.Start();
      sel.WordRight();
      sel.WordRight();
      sel.WordRight();
      sel.Delete(5);
      CPPUNIT_ASSERT(strcmp(text.c_str(), "This is the string") == 0);

      sel.Start();
      sel.WordRight();
      sel.Delete(3);
      CPPUNIT_ASSERT(strcmp(text.c_str(), "This the string") == 0);
   }

   ////////////////////////////////////

   void TestWordLeft()
   {
      char szTestString[] = "This is the test string";
      cUIString text(szTestString);
      cUITextSelection sel(&text);

      sel.End();
      sel.WordLeft();
      sel.Delete(6);
      CPPUNIT_ASSERT(strcmp(text.c_str(), "This is the test ") == 0);

      sel.WordLeft();
      sel.Delete(5);
      CPPUNIT_ASSERT(strcmp(text.c_str(), "This is the ") == 0);

      sel.WordLeft();
      sel.WordLeft();
      sel.WordLeft();
      sel.WordLeft();
      sel.Delete(5);
      CPPUNIT_ASSERT(strcmp(text.c_str(), "is the ") == 0);
   }

   ////////////////////////////////////

   void TestReplace()
   {
      char szTestString[] = "This is the test string";
      cUIString text(szTestString);
      cUITextSelection sel(&text);

      sel.Start();
      sel.WordRight();
      sel.Delete(2);
      sel.ReplaceSel("was");
      CPPUNIT_ASSERT(strcmp(text.c_str(), "This was the test string") == 0);

      sel.WordRight();
      sel.WordRight();
      sel.CharRight(4);
      sel.ReplaceSel("ing");
      CPPUNIT_ASSERT(strcmp(text.c_str(), "This was the testing string") == 0);
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cUITextSelectionTests);

#endif

///////////////////////////////////////////////////////////////////////////////
