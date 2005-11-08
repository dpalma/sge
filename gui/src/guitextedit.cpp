///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guitextedit.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guistrings.h"
#include "inputapi.h"
#include "sys.h"

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
// CLASS: cGUITextBuffer
//

///////////////////////////////////////

cGUITextBuffer::cGUITextBuffer()
{
   End();
}

///////////////////////////////////////

cGUITextBuffer::~cGUITextBuffer()
{
}

///////////////////////////////////////

tResult cGUITextBuffer::GetText(tGUIString * pText)
{
   if (pText == NULL)
   {
      return E_POINTER;
   }
   *pText = m_text;
   return pText->empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

tResult cGUITextBuffer::SetText(const char * pszText)
{
   if (pszText == NULL)
   {
      m_text.erase();
   }
   else
   {
      m_text.assign(pszText);
   }
   return S_OK;
}

///////////////////////////////////////

void cGUITextBuffer::SetCursorIndex(uint index)
{
   m_cursor = m_text.begin() + index;
}

///////////////////////////////////////

int cGUITextBuffer::GetCursorIndex() const
{
   return GetCursor() - m_text.begin();
}

///////////////////////////////////////

void cGUITextBuffer::CharRight(int count)
{
   while (count-- && m_cursor != m_text.end())
   {
      m_cursor++;
   }
}

///////////////////////////////////////

void cGUITextBuffer::CharLeft(int count)
{
   while (count-- && m_cursor != m_text.begin())
   {
      m_cursor--;
   }
}

///////////////////////////////////////

void cGUITextBuffer::WordRight()
{
   tGUIString::size_type start = GetCursor() - m_text.begin();

   tGUIString::size_type index = m_text.find_first_of(kWhiteSpaceChars, start);

   if (index != std::string::npos)
   {
      index = m_text.find_first_not_of(kWhiteSpaceChars, index);

      if (index != std::string::npos)
      {
         m_cursor = m_text.begin() + index;
      }
      else
      {
         m_cursor = m_text.end();
      }
   }
   else
   {
      m_cursor = m_text.end();
   }
}

///////////////////////////////////////

void cGUITextBuffer::WordLeft()
{
   tGUIString::size_type index = m_text.find_last_not_of(kWhiteSpaceChars, GetCursorIndex());

   if (index > 0 && index == GetCursorIndex())
   {
      index = m_text.find_last_of(kWhiteSpaceChars, index - 1);

      if (index > 0 && index != std::string::npos)
      {
         index = m_text.find_last_not_of(kWhiteSpaceChars, index - 1);
      }
   }

   if (index != std::string::npos)
   {
      index = m_text.find_last_of(kWhiteSpaceChars, index);

      if (index != std::string::npos)
      {
         m_cursor = m_text.begin() + index + 1;
      }
      else
      {
         m_cursor = m_text.begin();
      }
   }
}

///////////////////////////////////////

void cGUITextBuffer::Start()
{
   m_cursor = m_text.begin();
}

///////////////////////////////////////

void cGUITextBuffer::End()
{
   m_cursor = m_text.end();
}

///////////////////////////////////////

void cGUITextBuffer::Backspace(int count)
{
   while (count-- && m_cursor != m_text.begin())
   {
      m_cursor = m_text.erase(--m_cursor);
   }
}

///////////////////////////////////////

void cGUITextBuffer::Delete(int count)
{
   m_text.erase(GetCursor() - m_text.begin(), count);
}

///////////////////////////////////////

void cGUITextBuffer::ReplaceSel(char c)
{
   if (m_cursor == m_text.end())
   {
      m_text.append(1, c);
      m_cursor = m_text.end();
   }
   else
   {
      m_cursor = m_text.insert(m_cursor, c);
      m_cursor++;
   }
}

///////////////////////////////////////

void cGUITextBuffer::ReplaceSel(const char * psz)
{
   Assert(psz != NULL);
   if (m_cursor == m_text.end())
   {
      m_text.append(psz);
      m_cursor = m_text.end();
   }
   else
   {
#ifdef _DEBUG
      int preLength = m_text.length();
#endif
      m_text.insert(m_cursor - m_text.begin(), psz);
#ifdef _DEBUG
      int postLength = m_text.length();
      Assert(postLength == (preLength + strlen(psz)));
#endif
      m_cursor += strlen(psz);
   }
}

///////////////////////////////////////

void cGUITextBuffer::Cut()
{
   // TODO
}

///////////////////////////////////////

void cGUITextBuffer::Copy()
{
   // TODO
}

///////////////////////////////////////

void cGUITextBuffer::Paste()
{
   // TODO
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextEditElement
//

///////////////////////////////////////

cGUITextEditElement::cGUITextEditElement()
 : m_editSize(~0),
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
         m_buffer.SetCursorIndex(index);
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
      *pStart = m_buffer.GetCursorIndex();
   }
   if (pEnd != NULL)
   {
      *pEnd = m_buffer.GetCursorIndex();
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

   m_buffer.SetCursorIndex(end);

   return S_OK;
}

///////////////////////////////////////

tResult cGUITextEditElement::GetText(tGUIString * pText)
{
   return m_buffer.GetText(pText);
}

///////////////////////////////////////

tResult cGUITextEditElement::SetText(const char * pszText)
{
   return m_buffer.SetText(pszText);
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
         m_buffer.Backspace();
         break;
      }
      case kDelete:
      {
         m_buffer.Delete();
         break;
      }
      case kLeft:
      {
         UseGlobal(Input);
         if (pInput->KeyIsDown(kCtrl))
         {
            m_buffer.WordLeft();
         }
         else
         {
            m_buffer.CharLeft();
         }
         break;
      }
      case kRight:
      {
         UseGlobal(Input);
         if (pInput->KeyIsDown(kCtrl))
         {
            m_buffer.WordRight();               
         }
         else
         {
            m_buffer.CharRight();
         }
         break;
      }
      case kHome:
      {
         m_buffer.Start();
         break;
      }
      case kEnd:
      {
         m_buffer.End();
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
            m_buffer.ReplaceSel((char)keyCode);
         }
         break;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

bool cGUITextEditElement::HitTest(const tGUIPoint & point, int * pIndex)
{
   cAutoIPtr<IGUIElementRenderer> pRenderer;
   if (GetRenderer(&pRenderer) == S_OK)
   {
      tGUIString text;
      cAutoIPtr<IGUIFont> pFont;
      if (pRenderer->GetFont(static_cast<IGUIElement*>(this), &pFont) == S_OK
         && m_buffer.GetText(&text) == S_OK)
      {
         float charPos = 0;
         for (uint i = 0; i < text.length(); ++i)
         {
            tRect charRect(0,0,0,0);
            pFont->RenderText(&(text.at(i)), 1, &charRect, kRT_CalcRect, tGUIColor::White);

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
      }
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUITextEditElementCreate(const TiXmlElement * pXmlElement,
                                 IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementTextEdit) == 0)
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

AUTOREGISTER_GUIELEMENTFACTORYFN(textedit, GUITextEditElementCreate);


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cGUITextBufferTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGUITextBufferTests);
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

CPPUNIT_TEST_SUITE_REGISTRATION(cGUITextBufferTests);

///////////////////////////////////////

void cGUITextBufferTests::TestBackspace()
{
   char szTestString[] = "This is the test string";
   tGUIString text;
   cGUITextBuffer buffer;
   buffer.SetText(szTestString);

   int i;
   const int nBackspaces = 4;

   buffer.Start();
   for (i = 0; i < nBackspaces; i++)
   {
      buffer.Backspace();
   }
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

   buffer.End();
   for (i = 0; i < nBackspaces; i++)
   {
      buffer.Backspace();
   }
   szTestString[strlen(szTestString) - nBackspaces] = 0;
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);
}

///////////////////////////////////////

void cGUITextBufferTests::TestBackspace2()
{
   char szTestString[] = "This is the test string";
   tGUIString text;
   cGUITextBuffer buffer;
   buffer.SetText(szTestString);

   const int nBackspaces = 5;

   // Backspace at start should do nothing
   buffer.Start();
   buffer.Backspace(nBackspaces);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

   buffer.End();
   buffer.Backspace(nBackspaces);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strncmp(text.c_str(), szTestString, strlen(szTestString) - nBackspaces) == 0);
}

///////////////////////////////////////

void cGUITextBufferTests::TestBackspace3()
{
   char szTestString[] = "xxxHELLO";
   tGUIString text;
   cGUITextBuffer buffer;
   buffer.SetText(szTestString);

   const int nRight = 3;

   buffer.Start();
   buffer.CharRight(nRight);
   buffer.Backspace(nRight * 2);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString + nRight) == 0);
}

///////////////////////////////////////

void cGUITextBufferTests::TestDelete()
{
   char szTestString[] = "This is the test string";
   tGUIString text;
   cGUITextBuffer buffer;
   buffer.SetText(szTestString);

   int i;
   const int nDeletes = 4;

   // Delete from end should do nothing
   buffer.End();
   for (i = 0; i < nDeletes; i++)
   {
      buffer.Delete();
   }
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

   // Delete one-by-one from start
   buffer.Start();
   for (i = 0; i < nDeletes; i++)
   {
      buffer.Delete();
   }
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString + nDeletes) == 0);
}

///////////////////////////////////////

void cGUITextBufferTests::TestDelete2()
{
   char szTestString[] = "This is the test string";
   tGUIString text;
   cGUITextBuffer buffer;
   buffer.SetText(szTestString);

   const int nDeletes = 7;

   // Delete from end should do nothing
   buffer.End();
   buffer.Delete(nDeletes);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString) == 0);

   // Delete a # of characters from start
   buffer.Start();
   buffer.Delete(nDeletes);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), szTestString + nDeletes) == 0);
}

///////////////////////////////////////

void cGUITextBufferTests::TestWordRight()
{
   char szTestString[] = "This is the test string";
   tGUIString text;
   cGUITextBuffer buffer;
   buffer.SetText(szTestString);

   buffer.Start();
   buffer.WordRight();
   buffer.WordRight();
   buffer.WordRight();
   buffer.Delete(5);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), "This is the string") == 0);

   buffer.Start();
   buffer.WordRight();
   buffer.Delete(3);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), "This the string") == 0);
}

///////////////////////////////////////

void cGUITextBufferTests::TestWordLeft()
{
   char szTestString[] = "This is the test string";
   tGUIString text;
   cGUITextBuffer buffer;
   buffer.SetText(szTestString);

   buffer.End();
   buffer.WordLeft();
   buffer.Delete(6);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), "This is the test ") == 0);

   buffer.WordLeft();
   buffer.Delete(5);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), "This is the ") == 0);

   buffer.WordLeft();
   buffer.WordLeft();
   buffer.WordLeft();
   buffer.WordLeft();
   buffer.Delete(5);
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), "is the ") == 0);
}

///////////////////////////////////////

void cGUITextBufferTests::TestReplace()
{
   char szTestString[] = "This is the test string";
   tGUIString text;
   cGUITextBuffer buffer;
   buffer.SetText(szTestString);

   buffer.Start();
   buffer.WordRight();
   buffer.Delete(2);
   buffer.ReplaceSel("was");
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), "This was the test string") == 0);

   buffer.WordRight();
   buffer.WordRight();
   buffer.CharRight(4);
   buffer.ReplaceSel("ing");
   CPPUNIT_ASSERT(buffer.GetText(&text) == S_OK);
   CPPUNIT_ASSERT(strcmp(text.c_str(), "This was the testing string") == 0);
}

#endif

///////////////////////////////////////////////////////////////////////////////
