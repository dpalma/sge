///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guistylesheet.h"
#include "guielementbase.h"
#include "guielementbasetem.h"
#include "guielementtools.h"
#include "guiparse.h"
#include "guistrings.h"

#include "globalobj.h"

#include <tinyxml.h>

#include <cstring>
#include <locale>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyleSelector
//

////////////////////////////////////////

cGUIStyleSelector::cGUIStyleSelector(const tChar * pszSelector)
{
   if (pszSelector != NULL)
   {
      const tChar * pszDot = _tcschr(pszSelector, _T('.'));
      if (pszDot != NULL)
      {
         m_type = cStr(pszSelector, pszDot-pszSelector);
         m_class = cStr(pszDot+1);
      }
      else
      {
         m_type = pszSelector;
         m_class.erase();
      }
   }
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

bool cGUIStyleSelector::operator ==(const cGUIStyleSelector & other) const
{
   return m_type.compare(other.m_type) == 0 && m_class.compare(other.m_class) == 0;
}

////////////////////////////////////////

bool cGUIStyleSelector::operator <(const cGUIStyleSelector & other) const
{
   return m_type.compare(other.m_type) < 0 || m_class.compare(other.m_class) < 0;
}

////////////////////////////////////////

bool cGUIStyleSelector::operator >(const cGUIStyleSelector & other) const
{
   return m_type.compare(other.m_type) > 0 || m_class.compare(other.m_class) > 0;
}


///////////////////////////////////////////////////////////////////////////////
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

   m_styleMap[cGUIStyleSelector(pszSelector)] = CTAddRef(pStyle);
   return S_OK;
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
   if ((pszType == NULL && pszClass != NULL) || (pszType != NULL && pszClass == NULL))
   {
      return E_INVALIDARG;
   }

   // TODO: use the type

//   cGUIStyleSelector selector(pszType, pszClass);
   cGUIStyleSelector selector(NULL, pszClass);

   tStyleMap::const_iterator f = m_styleMap.find(selector);

   if (f != m_styleMap.end())
   {
      *ppStyle = CTAddRef(f->second);
      return S_OK;
   }

   // TODO: handle partial matches and over-rides

   return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

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
         styleSelector.TrimLeadingSpace();
         styleSelector.TrimTrailingSpace();

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

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyleElement
//

class cGUIStyleElement : public cComObject<cGUIElementBase<IGUIStyleElement>, &IID_IGUIStyleElement>
{
public:
   cGUIStyleElement(IGUIStyleSheet * pStyleSheet);
   ~cGUIStyleElement();

   // cGUIElementBase over-rides
   virtual tResult GetRendererClass(tGUIString * pRendererClass);
   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer);

   // IGUIStyleElement methods
   virtual tResult GetStyleSheet(IGUIStyleSheet * * ppStyleSheet) const;

private:
   cAutoIPtr<IGUIStyleSheet> m_pStyleSheet;
};

///////////////////////////////////////

cGUIStyleElement::cGUIStyleElement(IGUIStyleSheet * pStyleSheet)
 : m_pStyleSheet(CTAddRef(pStyleSheet))
{
}

///////////////////////////////////////

cGUIStyleElement::~cGUIStyleElement()
{
}

///////////////////////////////////////

tResult cGUIStyleElement::GetRendererClass(tGUIString * pRendererClass)
{
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyleElement::GetRenderer(IGUIElementRenderer * * ppRenderer)
{
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyleElement::GetStyleSheet(IGUIStyleSheet * * ppStyleSheet) const
{
   return m_pStyleSheet.GetPointer(ppStyleSheet);
}


///////////////////////////////////////////////////////////////////////////////

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
         const TiXmlNode * pFirstChild = pXmlElement->FirstChild();
         if (pFirstChild != NULL)
         {
            const TiXmlText * pText = pFirstChild->ToText();
            if (pText != NULL)
            {
               if (GUIStyleSheetParse(pText->Value(), &pStyleSheet) != S_OK)
               {
                  Assert(!pStyleSheet);
               }
            }
         }

         cAutoIPtr<IGUIStyleElement> pStyleElement = static_cast<IGUIStyleElement *>(new cGUIStyleElement(pStyleSheet));
         if (!pStyleElement)
         {
            return E_OUTOFMEMORY;
         }

         GUIElementStandardAttributes(pXmlElement, pStyleElement);

         *ppElement = CTAddRef(pStyleElement);
         return S_OK;
      }
   }
   else
   {
      *ppElement = static_cast<IGUIStyleElement *>(new cGUIStyleElement(NULL));
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(style, GUIStyleElementCreate);


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cGUIStyleSheetTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGUIStyleSheetTests);
      CPPUNIT_TEST(TestStyleSheetParse);
   CPPUNIT_TEST_SUITE_END();

   void TestStyleSheetParse();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGUIStyleSheetTests);

///////////////////////////////////////

void cGUIStyleSheetTests::TestStyleSheetParse()
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
   CPPUNIT_ASSERT(GUIStyleSheetParse(test, &pStyleSheet) == S_OK);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
