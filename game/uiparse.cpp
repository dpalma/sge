///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "techmath.h"
#include "parse.h"
#include "ui.h"
#include "uiparse.h"
#include "uiwidgets.h"
#include "uiwidgutilstem.h"
#include "uievent.h"
#include "readwriteapi.h"
#include "resmgr.h"
#include "globalobj.h"
#include <tinyxml.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#include <limits>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

typedef cUIComponent * (* tUIParseCreateFn)(const TiXmlElement &);

cUIComponent * UICreateComponent(const TiXmlElement & xmlElement);

#define kElemDialog     "dialog"
#define kElemLabel      "label"
#define kElemImage      "image"
#define kElemButton     "button"
#define kElemEdit       "edit"

#define kAttrId         "id"
#define kAttrText       "text"
#define kAttrSize       "size"
#define kAttrSrc        "src"
#define kAttrOnClick    "onclick"
#define kAttrTitle      "title"
#define kAttrLayout     "layout"

///////////////////////////////////////////////////////////////////////////////

cUIParseHook::~cUIParseHook()
{
}

///////////////////////////////////////////////////////////////////////////////

static bool UIParseColor(const char * psz, cUIColor * pColor)
{
   if (psz == NULL || *psz == 0 || pColor == NULL)
      return false;

   if (*psz == '#' && strlen(psz) == 7)
   {
      uint r,g,b;
      if (sscanf(psz + 1, "%02x%02x%02x", &r, &g, &b) == 3)
      {
         static const float kOneOver255 = 1.0f / 255.0f;
         *pColor = cUIColor(r*kOneOver255, g*kOneOver255, b*kOneOver255, 1);
         return true;
      }
   }
   else
   {
      double rgba[4] = { 0,0,0,1 };
      int nComponents = ParseTuple(psz, rgba, _countof(rgba));
      if (nComponents == 3 || nComponents == 4)
      {
         *pColor = cUIColor(rgba[0],rgba[1],rgba[2],rgba[3]);
         return true;
      }
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

template <class XMLTHING, class STLCONTAINER>
static void UIParseChildElements(const XMLTHING & xmlThing, STLCONTAINER * pComponents, cUIParseHook * pHook)
{
   TiXmlNode * pNode;
   for (pNode = xmlThing.FirstChild(); pNode != NULL; pNode = pNode->NextSibling())
   {
      if (pNode->Type() == TiXmlNode::ELEMENT)
      {
         eSkipResult skip = kNoSkip;

         if (pHook != NULL)
         {
            skip = pHook->SkipElement(pNode->Value());
         }

         if (skip == kSkipEntire)
         {
            continue;
         }
         else if (skip == kSkipAllowChildren) // @TODO: this is untested
         {
            UIParseChildElements(*pNode, pComponents, pHook);
            continue;
         }

         cUIComponent * pComponent = UICreateComponent(*pNode->ToElement());

         if (pComponent != NULL)
         {
            pComponents->push_back(pComponent);
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

cUIComponent * UICreateDialog(const TiXmlElement & xmlElement)
{
   if (strcmp(xmlElement.Value(), kElemDialog) != 0)
      return NULL;

   cUIDialog * pDialog = new cUIDialog;

   if (xmlElement.Attribute(kAttrTitle) != NULL)
      pDialog->SetTitle(xmlElement.Attribute(kAttrTitle));

   if (xmlElement.Attribute(kAttrLayout) != NULL)
   {
      double params[2];

      if (strnicmp("grid", xmlElement.Attribute(kAttrLayout), 4) == 0 &&
         ParseTuple(xmlElement.Attribute(kAttrLayout) + 4, params, _countof(params)))
      {
         pDialog->SetLayoutManager(UIGridLayoutManagerCreate(Round(params[0]), Round(params[1])));
      }
      else if (stricmp("flow", xmlElement.Attribute(kAttrLayout)) == 0)
      {
         pDialog->SetLayoutManager(UIFlowLayoutManagerCreate());
      }
   }

   double size[2];

   if (ParseTuple(xmlElement.Attribute(kAttrSize), size, _countof(size)) == 2)
      pDialog->SetSize(cUISize(size[0], size[1]));

   tUIComponentList children;
   UIParseChildElements(xmlElement, &children, NULL);

   tUIComponentList::iterator iter;
   for (iter = children.begin(); iter != children.end(); iter++)
   {
      pDialog->AddComponent(*iter);
   }

   pDialog->Layout();

   return pDialog;
}

///////////////////////////////////////////////////////////////////////////////

cUIComponent * UICreateLabel(const TiXmlElement & xmlElement)
{
   if (strcmp(xmlElement.Value(), kElemLabel) != 0)
      return NULL;

   cUILabel * pLabel = new cUILabel;

   TiXmlNode * pNode = xmlElement.FirstChild();
   if (pNode != NULL && pNode->Type() == TiXmlNode::TEXT)
   {
      pLabel->SetText(pNode->ToText()->Value());
   }

   return pLabel;
}

///////////////////////////////////////////////////////////////////////////////

cUIComponent * UICreateImage(const TiXmlElement & xmlElement)
{
   if (strcmp(xmlElement.Value(), kElemImage) != 0)
      return NULL;

   if (xmlElement.Attribute(kAttrSrc) != NULL)
   {
      cUIImage * pImage = new cUIImage;
      if (pImage->LoadImage(xmlElement.Attribute(kAttrSrc)))
         return pImage;
      delete pImage;
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////

cUIComponent * UICreateButton(const TiXmlElement & xmlElement)
{
   if (strcmp(xmlElement.Value(), kElemButton) != 0)
      return NULL;

   if (xmlElement.Attribute(kAttrSrc) != NULL)
   {
      cUIBitmapButton * pBitmapButton = new cUIBitmapButton;
      if (!pBitmapButton->SetBitmap(xmlElement.Attribute(kAttrSrc)))
      {
         delete pBitmapButton;
         return NULL;
      }

      TiXmlAttribute * pAttr = xmlElement.FirstAttribute();
      for (; pAttr != NULL; pAttr = pAttr->Next())
      {
         if (strnicmp(pAttr->Name(), "on", 2) == 0)
         {
            eUIEventCode code = UIEventCode(pAttr->Name() + 2);
            if (code != kEventERROR)
               pBitmapButton->SetEventHandler(code, pAttr->Value());
         }
      }

      return pBitmapButton;
   }
   else
   {
      cUIButton * pButton = new cUIButton;

      TiXmlAttribute * pAttr = xmlElement.FirstAttribute();
      for (; pAttr != NULL; pAttr = pAttr->Next())
      {
         if (stricmp(pAttr->Name(), kAttrText) == 0)
         {
            pButton->SetText(pAttr->Value());
         }
         else if (strnicmp(pAttr->Name(), "on", 2) == 0)
         {
            eUIEventCode code = UIEventCode(pAttr->Name() + 2);
            if (code != kEventERROR)
               pButton->SetEventHandler(code, pAttr->Value());
         }
      }

      return pButton;
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////

cUIComponent * UICreateEdit(const TiXmlElement & xmlElement)
{
   if (strcmp(xmlElement.Value(), kElemEdit) != 0)
      return NULL;

   cUIEdit * pEdit = new cUIEdit;

   if (xmlElement.Attribute(kAttrSize) != NULL)
      pEdit->SetSize(atoi(xmlElement.Attribute(kAttrSize)));

   return pEdit;
}

///////////////////////////////////////////////////////////////////////////////

static void AssignDefaultId(cUIComponent * pComponent, const char * pszPrefix)
{
   Assert(pComponent != NULL && pszPrefix != NULL);
   static ulong number = 0;
   char szId[50];
   sprintf(szId, "%s%d", pszPrefix, number++);
   pComponent->SetId(szId);
}

///////////////////////////////////////

static void GetStandardAttributes(cUIComponent * pComponent, const TiXmlElement & xmlElement)
{
   Assert(pComponent != NULL);
   if (xmlElement.Attribute(kAttrId) != NULL)
      pComponent->SetId(xmlElement.Attribute(kAttrId));
}

///////////////////////////////////////

cUIComponent * UICreateComponent(const TiXmlElement & xmlElement)
{
   static const struct
   {
      const char * type;
      tUIParseCreateFn pfn;
   }
   table[] =
   {
      { kElemDialog, UICreateDialog },
      { kElemLabel, UICreateLabel },
      { kElemImage, UICreateImage },
      { kElemButton, UICreateButton },
      { kElemEdit, UICreateEdit },
   };

   for (int i = 0; i < _countof(table); i++)
   {
      if (strcmp(xmlElement.Value(), table[i].type) == 0)
      {
         cUIComponent * pComponent = (*table[i].pfn)(xmlElement);

         if (pComponent != NULL)
         {
            GetStandardAttributes(pComponent, xmlElement);

            if (pComponent->GetId() == NULL || !*pComponent->GetId())
            {
               //DebugMsg2("%s component 0x%08x has no id attribute\n",
               //   table[i].type, pComponent);
               AssignDefaultId(pComponent, table[i].type);
            }
         }

         return pComponent;
      }
   }

   return NULL;
}

///////////////////////////////////////////////////////////////////////////////

template <class STLCONTAINER>
static bool UIParseString(const char * pszXml, STLCONTAINER * pComponents, cUIParseHook * pHook)
{
   Assert(pszXml != NULL);
   Assert(pComponents != NULL);

   TiXmlDocument doc;
   doc.Parse(pszXml);

   if (doc.Error())
   {
      DebugMsg1("Parse error: %s\n", doc.ErrorDesc());
      return false;
   }

   UIParseChildElements(doc, pComponents, pHook);

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static char * GetEntireContents(IReader * pReader)
{
   Assert(pReader != NULL);

   pReader->Seek(0, kSO_End);
   int length = pReader->Tell();
   pReader->Seek(0, kSO_Set);

   char * pszContents = new char[length + 1];

   if (pReader->Read(pszContents, length) != S_OK)
   {
      delete [] pszContents;
      return NULL;
   }

   pszContents[length] = 0;

   return pszContents;
}

int UIParseFile(const char * pszXmlFile, tUIComponentList * pComponents, cUIParseHook * pHook /*=NULL*/)
{
   UseGlobal(ResourceManager);
   cAutoIPtr<IReader> pReader = pResourceManager->Find(pszXmlFile);
   if (!pReader)
      return 0;

   // @TODO (dpalma 9-13-02): Make this a little smarter than reading
   // the entire file into memory.

   char * pszXml = GetEntireContents(pReader);
   if (pszXml == NULL)
      return 0;

   int result = UIParseString(pszXml, pComponents, pHook);

   delete [] pszXml;

   return result;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

const char * g_pszXmlParseTest =
{
   "<dialog bgcolor=\"(0.75,0.75,0.75,1)\" size=\"(250,200)\" >"
      "<label textcolor=\"(0,0,0,1)\">A label just for testing</label>"
      "<image src=\"image.tga\" />"
      "<bitmapbutton src=\"quitbutton.bmp\" onclick=\"quit();\" />"
   "</dialog>"
};

class cUITests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cUITests);
      CPPUNIT_TEST(TestParseXml);
      CPPUNIT_TEST(TestParseColor);
   CPPUNIT_TEST_SUITE_END();

public:
   void TestParseXml()
   {
      TiXmlDocument doc;
      doc.Parse(g_pszXmlParseTest);

      DebugMsgIf1(doc.Error(), "Parse error: %s\n", doc.ErrorDesc());
      CPPUNIT_ASSERT(!doc.Error());

      TiXmlNode * pNode;
      for (pNode = doc.FirstChild(); pNode != NULL; pNode = pNode->NextSibling())
      {
         if (pNode->Type() == TiXmlNode::ELEMENT)
         {
            delete UICreateComponent(*pNode->ToElement());
         }
      }
   }

   void TestParseColor()
   {
      cUIColor color;

      CPPUNIT_ASSERT(!UIParseColor("", &color));
      CPPUNIT_ASSERT(!UIParseColor(NULL, &color));
      CPPUNIT_ASSERT(!UIParseColor("blah", &color));

      CPPUNIT_ASSERT(UIParseColor("< .5 ; .2 ; .8 >", &color));
      CPPUNIT_ASSERT(fabs(color.GetRed() - .5f) < FLT_EPSILON);
      CPPUNIT_ASSERT(fabs(color.GetGreen() - .2f) < FLT_EPSILON);
      CPPUNIT_ASSERT(fabs(color.GetBlue() - .8f) < FLT_EPSILON);

      CPPUNIT_ASSERT(UIParseColor("#85A3C2", &color));
      CPPUNIT_ASSERT(fabs(color.GetRed() - 0.52156862745098039215686274509804f) < FLT_EPSILON);
      CPPUNIT_ASSERT(fabs(color.GetGreen() - 0.63921568627450980392156862745098f) < FLT_EPSILON);
      CPPUNIT_ASSERT(fabs(color.GetBlue() - 0.76078431372549019607843137254902f) < FLT_EPSILON);
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cUITests);

#endif

///////////////////////////////////////////////////////////////////////////////
