///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guielementtools.h"
#include "guistyle.h"

#include "parse.h"
#include "globalobj.h"

#include <tinyxml.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

tResult GUIElementCreateChildren(const TiXmlElement * pXmlElement, 
                                 IGUIContainerElement * pContainer)
{
   if (pXmlElement == NULL || pContainer == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   UseGlobal(GUIFactory);

   for (TiXmlElement * pXmlChild = pXmlElement->FirstChildElement(); 
        pXmlChild != NULL; pXmlChild = pXmlChild->NextSiblingElement())
   {
      if (pXmlChild->Type() == TiXmlNode::ELEMENT)
      {
         cAutoIPtr<IGUIElement> pChildElement;
         if (pGUIFactory->CreateElement(pXmlChild->Value(), pXmlChild, &pChildElement) == S_OK)
         {
            if ((result = pContainer->AddElement(pChildElement)) != S_OK)
            {
               DebugMsg("WARNING: Error creating child element\n");
               return result;
            }
         }
         else if (stricmp(pXmlChild->Value(), "layout") == 0)
         {
            cAutoIPtr<IGUILayoutManager> pLayout;
            if (GUILayoutManagerCreate(pXmlChild, &pLayout) == S_OK)
            {
               if ((result = pContainer->SetLayout(pLayout)) != S_OK)
               {
                  // Don't return the error result because the layout manager
                  // creating failing shouldn't be a fatal error
                  DebugMsg("WARNING: Error creating layout manager\n");
               }
            }
         }
      }
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIElementRenderChildren(IGUIContainerElement * pContainer,
                                 IRenderDevice * pRenderDevice)
{
   if (pContainer == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   cAutoIPtr<IGUIElementEnum> pEnum;
   if (pContainer->GetElements(&pEnum) == S_OK)
   {
      cAutoIPtr<IGUIElement> pChild;
      ulong count = 0;

      while ((pEnum->Next(1, &pChild, &count) == S_OK) && (count == 1))
      {
         if (pChild->IsVisible())
         {
            cAutoIPtr<IGUIElementRenderer> pChildRenderer;
            if (pChild->GetRenderer(&pChildRenderer) == S_OK)
            {
               if ((result = pChildRenderer->Render(pChild, pRenderDevice)) != S_OK)
               {
                  DebugMsg("WARNING: Error rendering child element\n");
                  return result;
               }
            }
         }

         SafeRelease(pChild);
         count = 0;
      }
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

void GUISizeElement(const tGUIRect & field, IGUIElement * pGUIElement)
{
   Assert(pGUIElement != NULL);

   tGUISize size(0, 0);

   cAutoIPtr<IGUIElementRenderer> pRenderer;
   if (pGUIElement->GetRenderer(&pRenderer) == S_OK)
   {
      size = pRenderer->GetPreferredSize(pGUIElement);
   }

   cAutoIPtr<IGUIStyle> pStyle;
   if (pGUIElement->GetStyle(&pStyle) == S_OK)
   {
      uint width, widthSpec;
      if (pStyle->GetWidth(&width, &widthSpec) == S_OK)
      {
         if (widthSpec == kGUIDimensionPixels)
         {
            size.width = width;
         }
         else if (widthSpec == kGUIDimensionPercent)
         {
            size.width = (width * field.GetWidth()) / 100;
         }
      }

      uint height, heightSpec;
      if (pStyle->GetHeight(&height, &heightSpec) == S_OK)
      {
         if (heightSpec == kGUIDimensionPixels)
         {
            size.height = height;
         }
         else if (heightSpec == kGUIDimensionPercent)
         {
            size.height = (height * field.GetHeight()) / 100;
         }
      }
   }

   pGUIElement->SetSize(size);
}

///////////////////////////////////////////////////////////////////////////////

void GUIPlaceElement(const tGUIRect & field, IGUIElement * pGUIElement)
{
   Assert(pGUIElement != NULL);

   tGUISize size = pGUIElement->GetSize();

   tGUIPoint pos(field.left, field.top);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pGUIElement->GetStyle(&pStyle) == S_OK)
   {
      uint align;
      if (pStyle->GetAlignment(&align) == S_OK)
      {
         if (align == kGUIAlignLeft)
         {
            pos.x = field.left;
         }
         else if (align == kGUIAlignRight)
         {
            pos.x = field.left + field.GetWidth() - size.width;
         }
         else if (align == kGUIAlignCenter)
         {
            pos.x = field.left + ((field.GetWidth() - size.width) / 2);
         }
      }

      uint vertAlign;
      if (pStyle->GetVerticalAlignment(&vertAlign) == S_OK)
      {
         if (vertAlign == kGUIVertAlignTop)
         {
            pos.y = field.top;
         }
         else if (vertAlign == kGUIVertAlignBottom)
         {
            pos.y = field.top + field.GetHeight() - size.height;
         }
         else if (vertAlign == kGUIVertAlignCenter)
         {
            pos.y = field.top + ((field.GetHeight() - size.height) / 2);
         }
      }
   }

   pGUIElement->SetPosition(pos);
}

///////////////////////////////////////////////////////////////////////////////

tGUIPoint GUIElementAbsolutePosition(IGUIElement * pGUIElement)
{
   Assert(pGUIElement != NULL);

   tGUIPoint absolutePosition = pGUIElement->GetPosition();

   cAutoIPtr<IGUIElement> pParent;
   if (pGUIElement->GetParent(&pParent) == S_OK)
   {
      do
      {
         absolutePosition += pParent->GetPosition();

         cAutoIPtr<IGUIElement> pNext;
         if (pParent->GetParent(&pNext) != S_OK)
         {
            SafeRelease(pParent);
         }
         else
         {
            pParent = pNext;
         }
      }
      while (!!pParent);
   }

   return absolutePosition;
}

///////////////////////////////////////////////////////////////////////////////

static bool StringToBool(const char * psz)
{
   Assert(psz != NULL);
   if (stricmp(psz, "true") == 0)
   {
      return true;
   }
   else if (strlen(psz) == 1)
   {
      if (psz[0] == 't' || psz[0] == 'T' || psz[0] == '1')
      {
         return true;
      }
   }
   return false;
}

tResult GUIElementStandardAttributes(const TiXmlElement * pXmlElement, 
                                     IGUIElement * pGUIElement)
{
   if (pXmlElement == NULL || pGUIElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement->Attribute("id"))
   {
      pGUIElement->SetId(pXmlElement->Attribute("id"));
   }

   if (pXmlElement->Attribute("visible"))
   {
      pGUIElement->SetVisible(StringToBool(pXmlElement->Attribute("visible")));
   }

   if (pXmlElement->Attribute("enabled"))
   {
      pGUIElement->SetEnabled(StringToBool(pXmlElement->Attribute("enabled")));
   }

   if (pXmlElement->Attribute("style"))
   {
      cAutoIPtr<IGUIStyle> pStyle;
      if (GUIStyleParse(pXmlElement->Attribute("style"), &pStyle) == S_OK)
      {
         pGUIElement->SetStyle(pStyle);
      }
   }

   cAutoIPtr<IGUIContainerElement> pContainer;
   if (pGUIElement->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
   {
      if (pXmlElement->Attribute("insets"))
      {
         double insetVals[4];
         if (ParseTuple(pXmlElement->Attribute("insets"), insetVals, _countof(insetVals)) == 4)
         {
            tGUIInsets insets;
            insets.left = Round(insetVals[0]);
            insets.top = Round(insetVals[1]);
            insets.right = Round(insetVals[2]);
            insets.bottom = Round(insetVals[3]);
            pContainer->SetInsets(insets);
         }
      }
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cGUIElementToolsTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGUIElementToolsTests);
      CPPUNIT_TEST(TestParseBool);
   CPPUNIT_TEST_SUITE_END();

   void TestParseBool();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGUIElementToolsTests);

///////////////////////////////////////

void cGUIElementToolsTests::TestParseBool()
{
   CPPUNIT_ASSERT(StringToBool("T"));
   CPPUNIT_ASSERT(StringToBool("t"));
   CPPUNIT_ASSERT(StringToBool("1"));
   CPPUNIT_ASSERT(StringToBool("true"));
   CPPUNIT_ASSERT(StringToBool("TRUE"));
   CPPUNIT_ASSERT(!StringToBool("F"));
   CPPUNIT_ASSERT(!StringToBool("f"));
   CPPUNIT_ASSERT(!StringToBool("0"));
   CPPUNIT_ASSERT(!StringToBool("false"));
   CPPUNIT_ASSERT(!StringToBool("FALSE"));
   CPPUNIT_ASSERT(!StringToBool("tttttt"));
   CPPUNIT_ASSERT(!StringToBool("abcdefg"));
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
