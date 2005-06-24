///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guielementtools.h"
#include "guistyle.h"
#include "guistrings.h"

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
               WarnMsg("Error creating child element\n");
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
                  WarnMsg("Error creating layout manager\n");
               }
            }
         }
      }
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIElementRenderChildren(IGUIContainerElement * pContainer)
{
   if (pContainer == NULL)
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
               result = pChildRenderer->Render(pChild);
               if (FAILED(result))
               {
                  SafeRelease(pChild);
                  break;
               }
            }
         }

         SafeRelease(pChild);
         count = 0;
      }
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIElementSizeFromStyle(IGUIElement * pElement,
                                const tGUISize & relativeTo,
                                tGUISize * pSize)
{
   if (pElement == NULL || pSize == NULL)
   {
      return E_POINTER;
   }

   bool bHaveWidth = false, bHaveHeight = false;
   tGUISizeType width, height;

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      uint styleWidth, styleWidthSpec;
      if (pStyle->GetWidth(&styleWidth, &styleWidthSpec) == S_OK)
      {
         if (styleWidthSpec == kGUIDimensionPixels)
         {
            width = static_cast<tGUISizeType>(styleWidth);
            bHaveWidth = true;
         }
         else if (styleWidthSpec == kGUIDimensionPercent)
         {
            width = static_cast<tGUISizeType>((styleWidth * relativeTo.width) / 100);
            bHaveWidth = true;
         }
      }

      uint styleHeight, styleHeightSpec;
      if (pStyle->GetHeight(&styleHeight, &styleHeightSpec) == S_OK)
      {
         if (styleHeightSpec == kGUIDimensionPixels)
         {
            height = static_cast<tGUISizeType>(styleHeight);
            bHaveHeight = true;
         }
         else if (styleHeightSpec == kGUIDimensionPercent)
         {
            height = static_cast<tGUISizeType>((styleHeight * relativeTo.height) / 100);
            bHaveHeight = true;
         }
      }
   }

   if (bHaveWidth && bHaveHeight)
   {
      *pSize = tGUISize(width, height);
      return S_OK;
   }

   return S_FALSE;
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

   tGUISize relTo(static_cast<tGUISizeType>(field.GetWidth()), static_cast<tGUISizeType>(field.GetHeight()));
   GUIElementSizeFromStyle(pGUIElement, relTo, &size);

   pGUIElement->SetSize(size);
}

///////////////////////////////////////////////////////////////////////////////

void GUIPlaceElement(const tGUIRect & field, IGUIElement * pGUIElement)
{
   Assert(pGUIElement != NULL);

   tGUISize size = pGUIElement->GetSize();

   tGUIPoint pos(static_cast<float>(field.left), static_cast<float>(field.top));

   cAutoIPtr<IGUIStyle> pStyle;
   if (pGUIElement->GetStyle(&pStyle) == S_OK)
   {
      uint align;
      if (pStyle->GetAlignment(&align) == S_OK)
      {
         if (align == kGUIAlignLeft)
         {
            pos.x = static_cast<float>(field.left);
         }
         else if (align == kGUIAlignRight)
         {
            pos.x = static_cast<float>(field.left + field.GetWidth() - size.width);
         }
         else if (align == kGUIAlignCenter)
         {
            pos.x = static_cast<float>(field.left + ((field.GetWidth() - size.width) / 2));
         }
      }

      uint vertAlign;
      if (pStyle->GetVerticalAlignment(&vertAlign) == S_OK)
      {
         if (vertAlign == kGUIVertAlignTop)
         {
            pos.y = static_cast<float>(field.top);
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

   if (pXmlElement->Attribute(kAttribId))
   {
      pGUIElement->SetId(pXmlElement->Attribute(kAttribId));
   }

   if (pXmlElement->Attribute(kAttribVisible))
   {
      pGUIElement->SetVisible(StringToBool(pXmlElement->Attribute(kAttribVisible)));
   }

   if (pXmlElement->Attribute(kAttribEnabled))
   {
      pGUIElement->SetEnabled(StringToBool(pXmlElement->Attribute(kAttribEnabled)));
   }

   if (pXmlElement->Attribute(kAttribStyle))
   {
      cAutoIPtr<IGUIStyle> pStyle;
      if (GUIStyleParse(pXmlElement->Attribute(kAttribStyle), &pStyle) == S_OK)
      {
         pGUIElement->SetStyle(pStyle);
      }
   }

   if (pXmlElement->Attribute(kAttribRendererClass))
   {
      pGUIElement->SetRendererClass(pXmlElement->Attribute(kAttribRendererClass));
   }

   cAutoIPtr<IGUIContainerElement> pContainer;
   if (pGUIElement->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
   {
      if (pXmlElement->Attribute(kAttribInsets))
      {
         const cStr insets(pXmlElement->Attribute(kAttribInsets));

         float insetVals[4];
         if (insets.ParseTuple(insetVals, _countof(insetVals)) == 4)
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
