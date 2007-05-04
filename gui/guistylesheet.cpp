////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guistylesheet.h"
#include "guielementbase.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guiparse.h"
#include "guistrings.h"

#include "tech/globalobj.h"
#include "tech/resourceapi.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <tinyxml.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>

#include <cstring>
#include <locale>

#include "tech/dbgalloc.h" // must be last header

using namespace boost;
using namespace std;

////////////////////////////////////////////////////////////////////////////////

static void ParseSelector(const tChar * pszSelector, cStr * pType, cStr * pClass)
{
   if (pszSelector != NULL)
   {
      const tChar * pszDot = _tcschr(pszSelector, _T('.'));
      if (pszDot != NULL)
      {
         *pType = cStr(pszSelector, pszDot-pszSelector);
         *pClass = cStr(pszDot+1);
      }
      else
      {
         *pType = pszSelector;
         pClass->erase();
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyleSelector
//

////////////////////////////////////////

cGUIStyleSelector::cGUIStyleSelector(const tChar * pszSelector)
{
   ParseSelector(pszSelector, &m_type, &m_class);
}

////////////////////////////////////////

cGUIStyleSelector::cGUIStyleSelector(const tChar * pszType, const tChar * pszClass)
 : m_type(pszType != NULL ? pszType : _T(""))
 , m_class(pszClass != NULL ? pszClass : _T(""))
{
}

////////////////////////////////////////

cGUIStyleSelector::cGUIStyleSelector(const cGUIStyleSelector & other)
 : m_type(other.m_type)
 , m_class(other.m_class)
{
}

////////////////////////////////////////

const cGUIStyleSelector & cGUIStyleSelector::operator =(const cGUIStyleSelector & other)
{
   m_type = other.m_type;
   m_class = other.m_class;
   return *this;
}

////////////////////////////////////////

bool cGUIStyleSelector::operator <(const cGUIStyleSelector & other) const
{
   if (m_type.empty() && other.m_type.empty())
   {
      return m_class.compare(other.m_class) < 0;
   }
   else if (m_type.empty() || other.m_type.empty())
   {
      return m_type.length() < other.m_type.length();
   }
   else
   {
      int typeCompare = m_type.compare(other.m_type);
      if (typeCompare == 0)
      {
         return m_class.compare(other.m_class) < 0;
      }
      return (typeCompare < 0);
   }
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyleSheet
//

////////////////////////////////////////

cGUIStyleSheet::cGUIStyleSheet()
{
}

////////////////////////////////////////

cGUIStyleSheet::~cGUIStyleSheet()
{
   tStyleMap::iterator iter = m_styleMap.begin();
   for (; iter != m_styleMap.end(); iter++)
   {
      iter->second->Release();
   }
   m_styleMap.clear();
}

////////////////////////////////////////

tResult cGUIStyleSheet::AddRule(const tChar * pszSelector, IGUIStyle * pStyle)
{
   if (pszSelector == NULL || pStyle == NULL)
   {
      return E_POINTER;
   }

   int nAdded = 0;

   cStr selector(pszSelector);
   typedef tokenizer<char_separator<tChar> > tokenizer;
   static const char_separator<tChar> sep(_T(","));
   tokenizer tokens(selector, sep);
   tokenizer::iterator iter = tokens.begin(), end = tokens.end();
   for (; iter != end; ++iter)
   {
      cStr s(*iter);
      trim(s);

      cStr type, cls;
      ParseSelector(s.c_str(), &type, &cls);

      if (!type.empty() && cls.empty())
      {
         m_styleMap[cGUIStyleSelector(type.c_str(), NULL)] = CTAddRef(pStyle);
         nAdded += 1;
      }

      if (type.empty() && !cls.empty())
      {
         m_styleMap[cGUIStyleSelector(NULL, cls.c_str())] = CTAddRef(pStyle);
         nAdded += 1;
      }

      if (!type.empty() && !cls.empty())
      {
         m_styleMap[cGUIStyleSelector(type.c_str(), cls.c_str())] = CTAddRef(pStyle);
         nAdded += 1;
      }
   }

   return (nAdded > 0) ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cGUIStyleSheet::GetStyle(const tChar * pszType, const tChar * pszClass,
                                 IGUIStyle * * ppStyle) const
{
   if (ppStyle == NULL)
   {
      return E_POINTER;
   }

   // One or the other may be NULL but not both
   if (pszType == NULL && pszClass == NULL)
   {
      return E_INVALIDARG;
   }

   tStyleMap::const_iterator f = m_styleMap.find(cGUIStyleSelector(pszType, pszClass));

   if (f == m_styleMap.end() && pszClass != NULL)
   {
      f = m_styleMap.find(cGUIStyleSelector(NULL, pszClass));
   }

   if (f == m_styleMap.end() && pszType != NULL)
   {
      f = m_styleMap.find(cGUIStyleSelector(pszType, NULL));
   }

   if (f == m_styleMap.end())
   {
      return S_FALSE;
   }

   *ppStyle = CTAddRef(f->second);
   return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

static void ExpungeCharacter(tChar c, cStr * pStr)
{
   cStr::iterator iter = pStr->begin(); 
   for (; iter != pStr->end(); iter++)
   {
      if (*iter == c)
      {
         iter = pStr->erase(iter);
      }
   }
}

////////////////////////////////////////////////////////////////////////////////

tResult GUIStyleSheetParse(const tChar * pszStyleSheet, IGUIStyleSheet * * ppStyleSheet)
{
   if (pszStyleSheet == NULL || ppStyleSheet == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIStyleSheet> pStyleSheet(new cGUIStyleSheet);
   if (!pStyleSheet)
   {
      return E_OUTOFMEMORY;
   }

   size_t length = _tcslen(pszStyleSheet);
   const tChar * pszIter = pszStyleSheet;
   const tChar * pszEnd = pszStyleSheet + length;
   while (pszIter < pszEnd)
   {
      pszIter = SkipSpaceFwd(pszIter);
      pszIter = SkipSingleComment(pszIter);
      pszIter = SkipSpaceFwd(pszIter);

      const tChar * pszStyleStart = _tcschr(pszIter, _T('{'));
      if (pszStyleStart != NULL)
      {
         cStr styleSelector(pszIter, pszStyleStart-pszIter);
         TrimLeadingSpace(&styleSelector);
         TrimTrailingSpace(&styleSelector);

         pszIter = pszStyleStart;

         const tChar * pszStyleEnd = _tcschr(pszStyleStart+1, _T('}'));
         if (pszStyleEnd != NULL)
         {
            pszIter = pszStyleEnd;

            cAutoIPtr<IGUIStyle> pStyle;
            if (GUIStyleParse(pszStyleStart+1, pszStyleEnd-pszStyleStart-1, &pStyle) == S_OK)
            {
               if (pStyleSheet->AddRule(styleSelector.c_str(), pStyle) != S_OK)
               {
                  ErrorMsg1("Error adding style rule \"%s\"\n", styleSelector.c_str());
                  return E_FAIL;
               }
            }
            else
            {
               cStr sample(pszStyleStart, Min(30,pszStyleEnd-pszStyleStart));
               sample.append(_T("..."));
               ExpungeCharacter(_T('\n'), &sample);
               ErrorMsg1("Error parsing style \"%s\"\n", sample.c_str());
               return E_FAIL;
            }
         }
      }

      pszIter += 1;
   }

   return pStyleSheet.GetPointer(ppStyleSheet);
}

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyleElement
//

class cGUIStyleElement : public cComObject<cGUIElementBase<IGUIStyleElement>, &IID_IGUIStyleElement>
{
public:
   cGUIStyleElement(IGUIStyleSheet * pStyleSheet);
   ~cGUIStyleElement();

   // cGUIElementBase over-rides
   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer);

   // IGUIStyleElement methods
   virtual tResult GetStyleSheet(IGUIStyleSheet * * ppStyleSheet) const;

private:
   cAutoIPtr<IGUIStyleSheet> m_pStyleSheet;
};

////////////////////////////////////////

cGUIStyleElement::cGUIStyleElement(IGUIStyleSheet * pStyleSheet)
 : m_pStyleSheet(CTAddRef(pStyleSheet))
{
}

////////////////////////////////////////

cGUIStyleElement::~cGUIStyleElement()
{
}

////////////////////////////////////////

tResult cGUIStyleElement::GetRenderer(IGUIElementRenderer * * ppRenderer)
{
   return S_FALSE;
}

////////////////////////////////////////

tResult cGUIStyleElement::GetStyleSheet(IGUIStyleSheet * * ppStyleSheet) const
{
   return m_pStyleSheet.GetPointer(ppStyleSheet);
}


////////////////////////////////////////////////////////////////////////////////

tResult GUIStyleElementCreate(const TiXmlElement * pXmlElement, IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementStyle) == 0)
      {
         cAutoIPtr<IGUIStyleSheet> pStyleSheet;

         if (pXmlElement->Attribute(kAttribSrc) != NULL)
         {
            if (pXmlElement->FirstChild() != NULL)
            {
               ErrorMsg1("Style element should either specify an external css "
                  "file in the %s attribute, or define styles in the body of "
                  "the element, but not both\n", kAttribSrc);
               return E_FAIL;
            }

            UseGlobal(ResourceManager);
            const char * pszText = NULL;
            if (pResourceManager->Load(pXmlElement->Attribute(kAttribSrc), kRT_Text, NULL, (void**)&pszText) == S_OK)
            {
               if (GUIStyleSheetParse(pszText, &pStyleSheet) != S_OK)
               {
                  return E_FAIL;
               }
            }
         }
         else
         {
            const TiXmlNode * pFirstChild = pXmlElement->FirstChild();
            if (pFirstChild != NULL)
            {
               const TiXmlText * pText = pFirstChild->ToText();
               if (pText != NULL)
               {
                  if (GUIStyleSheetParse(pText->Value(), &pStyleSheet) != S_OK)
                  {
                     return E_FAIL;
                  }
               }
            }
         }

         if (!pStyleSheet)
         {
            return E_FAIL;
         }

         cAutoIPtr<IGUIStyleElement> pStyleElement = static_cast<IGUIStyleElement *>(new cGUIStyleElement(pStyleSheet));
         if (!pStyleElement)
         {
            return E_OUTOFMEMORY;
         }

         *ppElement = CTAddRef(pStyleElement);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUIStyleElement *>(new cGUIStyleElement(NULL));
      return (*ppElement != NULL) ? S_OK : E_OUTOFMEMORY;
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(style, GUIStyleElementCreate);


////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

////////////////////////////////////////

TEST(GUIStyleSheetParse)
{
   static const tChar test[] =
   {
      "\n"
      "input{\n"
	   "  font-size: 12px;\n"
      "}\n"
      "select\n"
      "{\n"
	   "  font-size: 12px;\n"
      "}\n"
      "p\n"
      "{\n"
      "  color: #000000;\n"
      "  font-family: Verdana, Arial, Helvetica, sans-serif;\n"
      "  font-weight: normal;\n"
      "}\n"
   };
   cAutoIPtr<IGUIStyleSheet> pStyleSheet;
   CHECK(GUIStyleSheetParse(test, &pStyleSheet) == S_OK);
}

////////////////////////////////////////

static bool StylesAreEqual(IGUIStyle * pStyle1, IGUIStyle * pStyle2)
{
   Assert(pStyle1 && pStyle2);

#define STYLE_ATTRIB_EQUAL(Attrib, Type, Init) \
   do { \
      Type a = Init, b = Init; \
      tResult result1 = pStyle1->Get##Attrib(&a); \
      tResult result2 = pStyle2->Get##Attrib(&b); \
      ErrorMsgIf(result1 != result2, "Mismatching results getting " #Attrib "\n"); \
      if (a != b) \
         return false; \
   } while (0)

#define STYLE_ATTRIB_EQUAL_2(Attrib, Type1, Init1, Type2, Init2) \
   do { \
      Type1 a1 = Init1, b1 = Init1; \
      Type2 a2 = Init2, b2 = Init2; \
      tResult result1 = pStyle1->Get##Attrib(&a1, &a2); \
      tResult result2 = pStyle2->Get##Attrib(&b1, &b2); \
      ErrorMsgIf(result1 != result2, "Mismatching results getting " #Attrib "\n"); \
      if (a1 != b1 || a2 != b2) \
         return false; \
   } while (0)

   STYLE_ATTRIB_EQUAL(Alignment, uint, 0);
   STYLE_ATTRIB_EQUAL(VerticalAlignment, uint, 0);
   STYLE_ATTRIB_EQUAL(BackgroundColor, tGUIColor, tGUIColor(0,0,0,0));
   STYLE_ATTRIB_EQUAL(ForegroundColor, tGUIColor, tGUIColor(0,0,0,0));
   STYLE_ATTRIB_EQUAL(TextAlignment, uint, 0);
   STYLE_ATTRIB_EQUAL(TextVerticalAlignment, uint, 0);
   STYLE_ATTRIB_EQUAL(FontName, tGUIString, _T(""));
   STYLE_ATTRIB_EQUAL_2(FontSize, int, 0, uint, 0);
   STYLE_ATTRIB_EQUAL(FontBold, bool, false);
   STYLE_ATTRIB_EQUAL(FontItalic, bool, false);
   STYLE_ATTRIB_EQUAL(FontShadow, bool, false);
   STYLE_ATTRIB_EQUAL(FontOutline, bool, false);
   STYLE_ATTRIB_EQUAL(Placement, uint, 0);

   {
      int width1 = 0, width2 = 0;
      uint spec1 = 0, spec2 = 0;
      if (pStyle1->GetWidth(&width1, &spec1) != pStyle2->GetWidth(&width2, &spec2)
         || width1 != width2 || spec1 != spec2)
      {
         return false;
      }
   }

   {
      int height1 = 0, height2 = 0;
      uint spec1 = 0, spec2 = 0;
      if (pStyle1->GetHeight(&height1, &spec1) != pStyle2->GetHeight(&height2, &spec2)
         || height1 != height2 || spec1 != spec2)
      {
         return false;
      }
   }

   // TODO: test all the IDictionary-based custom attributes too

   // If made it this far, success
   return true;
}

TEST(GUIStyleSelectors)
{
   static const tChar test[] =
   {
      _T(
      "\n"
      "a.topText"
      "{\n"
	   "  font-size: 7pt;\n"
      "}\n"
      "a.doc\n"
      "{\n"
	   "  text-decoration: underline;\n"
      "}\n"
      ".heading\n"
      "{\n"
	   "  font-size: 18px;\n"
	   "  font-weight: bold;\n"
	   "  color: #B82619;\n"
      "}\n"
      "p\n"
      "{\n"
      "  color: #000000;\n"
      "  font-family: Verdana, Arial, Helvetica, sans-serif;\n"
      "  font-weight: normal;\n"
      "}\n"
      "h1,h2, h3, h4\n"
      "{\n"
      "  foreground-color: white;\n"
      "  background-color: blue;\n"
      "}\n"
      )
   };
   cAutoIPtr<IGUIStyleSheet> pStyleSheet;
   CHECK(GUIStyleSheetParse(test, &pStyleSheet) == S_OK);

   {
      cAutoIPtr<IGUIStyle> pStyle;
      CHECK(pStyleSheet->GetStyle(_T("a"), _T("topText"), &pStyle) == S_OK);
      int i;
      uint u;
      CHECK(pStyle->GetFontSize(&i, &u) == S_OK);
      CHECK_EQUAL(i, 7);
      CHECK_EQUAL(u, kGUIFontSizePoints);
      CHECK(pStyle->GetAlignment(&u) == S_FALSE);
   }

   {
      cAutoIPtr<IGUIStyle> pStyle;
      CHECK(pStyleSheet->GetStyle(NULL, _T("heading"), &pStyle) == S_OK);
   }

   {
      cAutoIPtr<IGUIStyle> pStyle;
      CHECK(pStyleSheet->GetStyle(_T("p"), NULL, &pStyle) == S_OK);
   }

   {
      cAutoIPtr<IGUIStyle> pStyle1, pStyle2, pStyle3;
      CHECK(pStyleSheet->GetStyle(_T("h1"), NULL, &pStyle1) == S_OK);
      CHECK(pStyleSheet->GetStyle(_T("h2"), NULL, &pStyle2) == S_OK);
      CHECK(pStyleSheet->GetStyle(_T("h3"), NULL, &pStyle3) == S_OK);
      CHECK(StylesAreEqual(pStyle1, pStyle2));
      CHECK(StylesAreEqual(pStyle2, pStyle3));
   }
}

#endif // HAVE_UNITTESTPP

////////////////////////////////////////////////////////////////////////////////
