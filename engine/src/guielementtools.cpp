///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guielementtools.h"
#include "guistrings.h"

#include "globalobj.h"

#include <tinyxml.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

bool GUIElementIdMatch(IGUIElement * pElement, const tChar * pszId)
{
   if (pElement != NULL && pszId != NULL)
   {
      tGUIString id;
      if (pElement->GetId(&id) == S_OK && id.compare(pszId) == 0)
      {
         return true;
      }
   }
   return false;
}

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
         if (pGUIFactory->CreateElement(pXmlChild, &pChildElement) == S_OK)
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

tResult GUIElementRenderChildren(IGUIContainerElement * pContainer, IGUIRenderDevice * pRenderDevice)
{
   if (pContainer == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIElementEnum> pEnum;
   if (pContainer->GetElements(&pEnum) == S_OK)
   {
      IGUIElement * pChildren[32];
      ulong count = 0;

      while (SUCCEEDED((pEnum->Next(_countof(pChildren), &pChildren[0], &count))) && (count > 0))
      {
         for (ulong i = 0; i < count; i++)
         {
            if (pChildren[i]->IsVisible())
            {
               cAutoIPtr<IGUIElementRenderer> pChildRenderer;
               if (pChildren[i]->GetRenderer(&pChildRenderer) == S_OK)
               {
                  if (FAILED(pChildRenderer->Render(pChildren[i], pRenderDevice)))
                  {
                     ErrorMsg("An error occured rendering a child GUI element\n");
                     for (ulong j = i; j < count; j++)
                     {
                        SafeRelease(pChildren[j]);
                     }
                     return E_FAIL;
                  }
               }
            }

            SafeRelease(pChildren[i]);
         }

         count = 0;
      }

      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUISizeElement(IGUIElement * pElement, const tGUISize & relativeTo)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   bool bHavePreferred = false, bHaveStyle = false;

   tGUISize size(0,0);

   cAutoIPtr<IGUIElementRenderer> pRenderer;
   if (pElement->GetRenderer(&pRenderer) == S_OK)
   {
      size = pRenderer->GetPreferredSize(pElement);
      bHavePreferred = true;
   }

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      uint styleWidth, styleWidthSpec;
      if (pStyle->GetWidth(&styleWidth, &styleWidthSpec) == S_OK)
      {
         if (styleWidthSpec == kGUIDimensionPixels)
         {
            size.width = static_cast<tGUISizeType>(styleWidth);
            bHaveStyle = true;
         }
         else if (styleWidthSpec == kGUIDimensionPercent)
         {
            size.width = static_cast<tGUISizeType>((styleWidth * relativeTo.width) / 100);
            bHaveStyle = true;
         }
      }

      uint styleHeight, styleHeightSpec;
      if (pStyle->GetHeight(&styleHeight, &styleHeightSpec) == S_OK)
      {
         if (styleHeightSpec == kGUIDimensionPixels)
         {
            size.height = static_cast<tGUISizeType>(styleHeight);
            bHaveStyle = true;
         }
         else if (styleHeightSpec == kGUIDimensionPercent)
         {
            size.height = static_cast<tGUISizeType>((styleHeight * relativeTo.height) / 100);
            bHaveStyle = true;
         }
      }
   }

   if (bHavePreferred || bHaveStyle)
   {
      pElement->SetSize(size);
      return  S_OK;
   }
   else
   {
      return S_FALSE;
   }
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

tGUIPoint GUIElementAbsolutePosition(IGUIElement * pGUIElement, uint * pnParents)
{
   Assert(pGUIElement != NULL);

   uint nParents = 0;
   tGUIPoint absolutePosition = pGUIElement->GetPosition();

   cAutoIPtr<IGUIElement> pParent;
   if (pGUIElement->GetParent(&pParent) == S_OK)
   {
      do
      {
         absolutePosition += pParent->GetPosition();
         nParents++;

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

   if (pnParents != NULL)
   {
      *pnParents = nParents;
   }

   return absolutePosition;
}

///////////////////////////////////////////////////////////////////////////////

extern tResult GUIStyleParseBool(const char * psz, bool * pBool);

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

   bool bBoolValue;
   if (GUIStyleParseBool(pXmlElement->Attribute(kAttribVisible), &bBoolValue) == S_OK)
   {
      pGUIElement->SetVisible(bBoolValue);
   }

   if (GUIStyleParseBool(pXmlElement->Attribute(kAttribEnabled), &bBoolValue) == S_OK)
   {
      pGUIElement->SetEnabled(bBoolValue);
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
//
// CLASS: cSizeAndPlaceElement
//

////////////////////////////////////////

cSizeAndPlaceElement::cSizeAndPlaceElement(const tGUIRect & rect)
 : m_rect(rect),
   m_size(static_cast<tGUISizeType>(m_rect.GetWidth()), static_cast<tGUISizeType>(m_rect.GetHeight()))
{
}

////////////////////////////////////////

tResult cSizeAndPlaceElement::operator()(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }
   GUISizeElement(pElement, m_size);
   GUIPlaceElement(m_rect, pElement);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cGUIElementToolsTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGUIElementToolsTests);
   CPPUNIT_TEST_SUITE_END();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGUIElementToolsTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
