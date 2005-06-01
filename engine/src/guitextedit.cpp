///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guitextedit.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"

#include "inputapi.h"

#include "font.h"
#include "color.h"
#include "renderapi.h"

#include "globalobj.h"
#include "techtime.h"
#include "keys.h"

#include <tinyxml.h>
#include <locale>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GUITextEditEvents);

#define LocalMsg(msg)                     DebugMsgEx(GUITextEditEvents,(msg))
#define LocalMsg1(msg,arg1)               DebugMsgEx1(GUITextEditEvents,(msg),(arg1))
#define LocalMsg2(msg,arg1,arg2)          DebugMsgEx2(GUITextEditEvents,(msg),(arg1),(arg2))
#define LocalMsg3(msg,arg1,arg2,arg3)     DebugMsgEx3(GUITextEditEvents,(msg),(arg1),(arg2),(arg3))

static const char kWhiteSpaceChars[] = " \n\r\t";

static const int kCursorBlinkFreq = 2; // blink this many times per second
static const float kCursorBlinkPeriod = 1.f / kCursorBlinkFreq;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextSelection
//

///////////////////////////////////////

cGUITextSelection::cGUITextSelection(tGUIString * pText)
 : m_pText(pText)
{
   End();
}

///////////////////////////////////////

cGUITextSelection::~cGUITextSelection()
{
   m_pText = NULL;
}

///////////////////////////////////////

void cGUITextSelection::SetCursorIndex(uint index)
{
   m_cursor = GetText()->begin() + index;
}

///////////////////////////////////////

int cGUITextSelection::GetCursorIndex() const
{
   return GetCursor() - GetText()->begin();
}

///////////////////////////////////////

void cGUITextSelection::CharRight(int count)
{
   while (count-- && m_cursor != GetText()->end())
   {
      m_cursor++;
   }
}

///////////////////////////////////////

void cGUITextSelection::CharLeft(int count)
{
   while (count-- && m_cursor != GetText()->begin())
   {
      m_cursor--;
   }
}

///////////////////////////////////////

void cGUITextSelection::WordRight()
{
   tGUIString::size_type start = GetCursor() - GetText()->begin();

   tGUIString::size_type index = GetText()->find_first_of(kWhiteSpaceChars, start);

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

void cGUITextSelection::WordLeft()
{
   tGUIString::size_type index = GetText()->find_last_not_of(kWhiteSpaceChars, GetCursorIndex());

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

void cGUITextSelection::Start()
{
   m_cursor = GetText()->begin();
}

///////////////////////////////////////

void cGUITextSelection::End()
{
   m_cursor = GetText()->end();
}

///////////////////////////////////////

void cGUITextSelection::Backspace(int count)
{
   while (count-- && m_cursor != GetText()->begin())
   {
      m_cursor = GetText()->erase(--m_cursor);
   }
}

///////////////////////////////////////

void cGUITextSelection::Delete(int count)
{
   GetText()->erase(GetCursor() - GetText()->begin(), count);
}

///////////////////////////////////////

void cGUITextSelection::ReplaceSel(char c)
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

void cGUITextSelection::ReplaceSel(const char * psz)
{
   Assert(psz != NULL);
   if (m_cursor == GetText()->end())
   {
      GetText()->append(psz);
      m_cursor = GetText()->end();
   }
   else
   {
#ifdef _DEBUG
      int preLength = GetText()->length();
#endif
      GetText()->insert(m_cursor - GetText()->begin(), psz);
#ifdef _DEBUG
      int postLength = GetText()->length();
      Assert(postLength == (preLength + strlen(psz)));
#endif
      m_cursor += strlen(psz);
   }
}

///////////////////////////////////////

void cGUITextSelection::Cut()
{
   // TODO
}

///////////////////////////////////////

void cGUITextSelection::Copy()
{
   // TODO
}

///////////////////////////////////////

void cGUITextSelection::Paste()
{
   // TODO
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextEditElement
//

///////////////////////////////////////

cGUITextEditElement::cGUITextEditElement()
 : m_text(""),
   m_editSize(~0),
   m_selection(&m_text),
   m_timeLastBlink(0),
   m_bCursorBlinkOn(true),
   m_bCursorForceOn(false)
{
}

///////////////////////////////////////

cGUITextEditElement::~cGUITextEditElement()
{
}

///////////////////////////////////////

tResult cGUITextEditElement::OnEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   if (eventCode == kGUIEventMouseEnter)
   {
   }
   else if (eventCode == kGUIEventMouseLeave)
   {
   }
   else if (eventCode == kGUIEventMouseDown)
   {
      tGUIPoint mouse;
      Verify(pEvent->GetMousePosition(&mouse) == S_OK);

      int index = -1;
      if (HitTest(mouse - GetAbsolutePosition(), &index) && (index != -1))
      {
         LocalMsg1("Hit index %d\n", index);
         m_selection.SetCursorIndex(index);
      }
   }
   else if (eventCode == kGUIEventMouseUp)
   {
   }
   else if (eventCode == kGUIEventKeyDown)
   {
      long keyCode;
      Verify(pEvent->GetKeyCode(&keyCode) == S_OK);

      // Attempt to keep the cursor visible while the user is holding down a key
      m_bCursorForceOn = 
         (keyCode != kCtrl) && 
         (keyCode != kLShift) && 
         (keyCode != kRShift);

      return HandleKeyDown(keyCode);
   }
   else if (eventCode == kGUIEventKeyUp)
   {
      m_bCursorForceOn = false;
      m_bCursorBlinkOn = true;
      m_timeLastBlink = TimeGetSecs();
   }
   else if (eventCode == kGUIEventClick)
   {
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUITextEditElement::GetEditSize(uint * pEditSize)
{
   if (pEditSize == NULL)
   {
      return E_POINTER;
   }

   if (m_editSize == -1)
   {
      return S_FALSE;
   }

   *pEditSize = m_editSize;

   return S_OK;
}

///////////////////////////////////////

tResult cGUITextEditElement::SetEditSize(uint editSize)
{
   m_editSize = editSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUITextEditElement::GetSelection(uint * pStart, uint * pEnd)
{
   // TODO: see comments below about text selection
   if (pStart != NULL)
   {
      *pStart = m_selection.GetCursorIndex();
   }
   if (pEnd != NULL)
   {
      *pEnd = m_selection.GetCursorIndex();
   }
   return S_OK;
}

///////////////////////////////////////

tResult cGUITextEditElement::SetSelection(uint start, uint end)
{
   if (start != end)
   {
      // TODO: true selection not supported yet, so this function can only
      // really be used to set the cursor position
      return E_NOTIMPL;
   }

   m_selection.SetCursorIndex(end);

   return S_OK;
}

///////////////////////////////////////

const char * cGUITextEditElement::GetText() const
{
   return m_text.c_str();
}

///////////////////////////////////////

void cGUITextEditElement::SetText(const char * pszText)
{
   m_text = pszText;
   m_selection.Start();
}

///////////////////////////////////////

void cGUITextEditElement::UpdateBlinkingCursor()
{
   double time = TimeGetSecs();
   if (m_timeLastBlink == 0)
   {
      m_timeLastBlink = time;
   }
   else if ((time - m_timeLastBlink) > kCursorBlinkPeriod)
   {
      m_bCursorBlinkOn = !m_bCursorBlinkOn;
      m_timeLastBlink = time;
   }
}

///////////////////////////////////////

bool cGUITextEditElement::ShowBlinkingCursor() const
{
   return m_bCursorBlinkOn || m_bCursorForceOn;
}

///////////////////////////////////////

tResult cGUITextEditElement::HandleKeyDown(long keyCode)
{
   switch (keyCode)
   {
      case kBackspace:
      {
         m_selection.Backspace();
         break;
      }
      case kDelete:
      {
         m_selection.Delete();
         break;
      }
      case kLeft:
      {
         UseGlobal(Input);
         if (pInput->KeyIsDown(kCtrl))
         {
            m_selection.WordLeft();
         }
         else
         {
            m_selection.CharLeft();
         }
         break;
      }
      case kRight:
      {
         UseGlobal(Input);
         if (pInput->KeyIsDown(kCtrl))
         {
            m_selection.WordRight();               
         }
         else
         {
            m_selection.CharRight();
         }
         break;
      }
      case kHome:
      {
         m_selection.Start();
         break;
      }
      case kEnd:
      {
         m_selection.End();
         break;
      }
      case kEscape:
      {
         // don't eat escape keypresses
         return S_OK;
      }
      default:
      {
         if (isprint(keyCode))
         {
            m_selection.ReplaceSel((char)keyCode);
         }
         break;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

bool cGUITextEditElement::HitTest(const tGUIPoint & point, int * pIndex)
{
   cAutoIPtr<IRenderFont> pFont;

   cAutoIPtr<IGUIStyle> pStyle;
   if (GetStyle(&pStyle) == S_OK)
   {
      pStyle->GetFont(&pFont);
   }

   if (!pFont)
   {
      UseGlobal(GUIRenderingTools);
      pGUIRenderingTools->GetDefaultFont(&pFont);
   }

   float charPos = 0;
   for (uint i = 0; i < m_text.length(); ++i)
   {
      tRect charRect(0,0,0,0);
      pFont->DrawText(&(m_text.at(i)), 1, kDT_CalcRect, &charRect, tGUIColor::White);

      if ((point.x > charPos) && (point.x <= charPos + charRect.GetWidth()))
      {
         if (pIndex != NULL)
         {
            *pIndex = i;
         }
         return true;
      }

      charPos += charRect.GetWidth();
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextEditElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(textedit, cGUITextEditElementFactory);

tResult cGUITextEditElementFactory::CreateElement(const TiXmlElement * pXmlElement, 
                                                  IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), "textedit") == 0)
      {
         cAutoIPtr<IGUITextEditElement> pTextEdit = static_cast<IGUITextEditElement *>(new cGUITextEditElement);
         if (!!pTextEdit)
         {
            GUIElementStandardAttributes(pXmlElement, pTextEdit);

            if (pXmlElement->Attribute(kAttribText))
            {
               pTextEdit->SetText(pXmlElement->Attribute(kAttribText));
            }

            if (pXmlElement->Attribute(kAttribEditSize))
            {
               uint editSize;
               if (sscanf(pXmlElement->Attribute(kAttribEditSize), "%d", &editSize) == 1)
               {
                  pTextEdit->SetEditSize(editSize);
               }
            }

            *ppElement = CTAddRef(pTextEdit);
            return S_OK;
         }
      }
   }
   else
   {
      *ppElement = static_cast<IGUITextEditElement *>(new cGUITextEditElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cGUITextSelectionTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGUITextSelectionTests);
      CPPUNIT_TEST(TestBackspace);
      CPPUNIT_TEST(TestBackspace2);
      CPPUNIT_TEST(TestBackspace3);
      CPPUNIT_TEST(TestDelete);
      CPPUNIT_TEST(TestDelete2);
      CPPUNIT_TEST(TestWordRight);
      CPPUNIT_TEST(TestWordLeft);
      CPPUNIT_TEST(TestReplace);
   CPPUNIT_TEST_SUITE_END();

   void TestBackspace();
   void TestBackspace2();
   void TestBackspace3();
   void TestDelete();
   void TestDelete2();
   void TestWordRight();
   void TestWordLeft();
   void TestReplace();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGUITextSelectionTests);

///////////////////////////////////////

void cGUITextSelectionTests::TestBackspace()
{
   char szTestString[] = "This is the test string";
   tGUIString text(szTestString);
   cGUITextSelection sel(&text);

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

///////////////////////////////////////

void cGUITextSelectionTests::TestBackspace2()
{
   char szTestString[] = "This is the test string";
   tGUIString text(szTestString);
   cGUITextSelection sel(&text);

   const int nBackspaces = 5;

   sel.Start();
   sel.Backspace(nBackspaces);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

   sel.End();
   sel.Backspace(nBackspaces);
   szTestString[strlen(szTestString) - nBackspaces] = 0;
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);
}

///////////////////////////////////////

void cGUITextSelectionTests::TestBackspace3()
{
   tGUIString text("xxxHELLO");
   cGUITextSelection sel(&text);

   sel.Start();
   sel.CharRight(3);
   sel.Backspace(5);
   CPPUNIT_ASSERT(strcmp(text.c_str(), "HELLO") == 0);
}

///////////////////////////////////////

void cGUITextSelectionTests::TestDelete()
{
   char szTestString[] = "This is the test string";
   tGUIString text(szTestString);
   cGUITextSelection sel(&text);

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

///////////////////////////////////////

void cGUITextSelectionTests::TestDelete2()
{
   char szTestString[] = "This is the test string";
   tGUIString text(szTestString);
   cGUITextSelection sel(&text);

   const int nDeletes = 7;

   sel.End();
   sel.Delete(nDeletes);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

   sel.Start();
   sel.Delete(nDeletes);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString + nDeletes) == 0);
}

///////////////////////////////////////

void cGUITextSelectionTests::TestWordRight()
{
   char szTestString[] = "This is the test string";
   tGUIString text(szTestString);
   cGUITextSelection sel(&text);

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

///////////////////////////////////////

void cGUITextSelectionTests::TestWordLeft()
{
   char szTestString[] = "This is the test string";
   tGUIString text(szTestString);
   cGUITextSelection sel(&text);

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

///////////////////////////////////////

void cGUITextSelectionTests::TestReplace()
{
   char szTestString[] = "This is the test string";
   tGUIString text(szTestString);
   cGUITextSelection sel(&text);

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

#endif

///////////////////////////////////////////////////////////////////////////////
