///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guielementtools.h"
#include "guiapi.h"
#include "guiparse.h"
#include "guistrings.h"
#include "guistyleapi.h"

#include "globalobj.h"

#include <tinyxml.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

bool IsDescendant(IGUIElement * pParent, IGUIElement * pElement)
{
   Assert(pParent != NULL);
   Assert(pElement != NULL);

   cAutoIPtr<IGUIElement> pTestParent;

   if (pElement->GetParent(&pTestParent) == S_OK)
   {
      do
      {
         if (CTIsSameObject(pTestParent, pParent))
         {
            return true;
         }

         cAutoIPtr<IGUIElement> pNextParent;
         if (pTestParent->GetParent(&pNextParent) != S_OK)
         {
            SafeRelease(pTestParent);
         }
         else
         {
            pTestParent = pNextParent;
         }
      }
      while (!!pTestParent);
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIElementType(IUnknown * pUnkElement, cStr * pType)
{
   if (pUnkElement == NULL || pType == NULL)
   {
      return E_POINTER;
   }

   static const struct
   {
      const GUID * pIID;
      const tChar * pszType;
   }
   guiElementTypes[] =
   {
      { &IID_IGUIButtonElement,     _T("Button") },
      { &IID_IGUIContainerElement,  _T("Container") },
      { &IID_IGUIDialogElement,     _T("Dialog") },
      { &IID_IGUILabelElement,      _T("Label") },
      { &IID_IGUIListBoxElement,    _T("ListBox") },
      { &IID_IGUIPanelElement,      _T("Panel") },
      { &IID_IGUIScriptElement,     _T("Script") },
      { &IID_IGUIScrollBarElement,  _T("ScrollBar") },
      { &IID_IGUIStyleElement,      _T("Style") },
      { &IID_IGUITextEditElement,   _T("TextEdit") },
   };

   for (int i = 0; i < _countof(guiElementTypes); i++)
   {
      cAutoIPtr<IUnknown> pUnk;
      if (pUnkElement->QueryInterface(*guiElementTypes[i].pIID, (void**)&pUnk) == S_OK)
      {
         *pType = guiElementTypes[i].pszType;
         return S_OK;
      }
   }

   return S_FALSE;
}


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
      if (pRenderer->GetPreferredSize(pElement, &size) == S_OK)
      {
         bHavePreferred = true;
      }
   }

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      int styleWidth;
      uint styleWidthSpec;
      if (pStyle->GetWidth(&styleWidth, &styleWidthSpec) == S_OK && styleWidth >= 0)
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

      int styleHeight;
      uint styleHeightSpec;
      if (pStyle->GetHeight(&styleHeight, &styleHeightSpec) == S_OK && styleHeight >= 0)
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
      tGUISize elementSize(Min(size.width, relativeTo.width), Min(size.height, relativeTo.height));
      pElement->SetSize(elementSize);
      tGUIRect clientArea(0,0,0,0);
      if (elementSize.width > 0 && elementSize.height > 0)
      {
         pRenderer->ComputeClientArea(pElement, &clientArea);
      }
      pElement->SetClientArea(clientArea);
      return S_OK;
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

template <typename INTRFC, const IID * PIID>
static tResult HackGetFirstElement(IGUIElementEnum * pEnum, INTRFC * * ppElement)
{
   if (pEnum == NULL || ppElement == NULL)
   {
      return E_POINTER;
   }
   cAutoIPtr<IGUIElement> pElement;
   ulong count = 0, total = 0;
   if (SUCCEEDED((pEnum->Next(1, &pElement, &count))) && (count == 1))
   {
      return pElement->QueryInterface(*PIID, (void**)ppElement);
   }
   return E_FAIL;
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

   bool bBoolValue;
   if (GUIParseBool(pXmlElement->Attribute(kAttribVisible), &bBoolValue) == S_OK)
   {
      pGUIElement->SetVisible(bBoolValue);
   }

   if (GUIParseBool(pXmlElement->Attribute(kAttribEnabled), &bBoolValue) == S_OK)
   {
      pGUIElement->SetEnabled(bBoolValue);
   }

   cAutoIPtr<IGUIStyle> pClassStyle, pInlineStyle;

   {
      UseGlobal(GUIContext);
      cAutoIPtr<IGUIElementEnum> pEnum;
      if (pGUIContext->GetElementsOfType(IID_IGUIStyleElement, &pEnum) == S_OK)
      {
         // TODO: how to handle multiple style sheets on the same page?
         cAutoIPtr<IGUIStyleElement> pStyleElement;
         if (HackGetFirstElement<IGUIStyleElement, &IID_IGUIStyleElement>(pEnum, &pStyleElement) == S_OK)
         {
            cAutoIPtr<IGUIStyleSheet> pStyleSheet;
            if (pStyleElement->GetStyleSheet(&pStyleSheet) == S_OK)
            {
               pStyleSheet->GetStyle(pXmlElement->Value(),
                  pXmlElement->Attribute(kAttribStyleClass), &pClassStyle);
            }
         }
      }
   }

   {
      const char * pszStyleAttrib = pXmlElement->Attribute(kAttribStyle);
      if (pszStyleAttrib != NULL)
      {
         GUIStyleParseInline(pszStyleAttrib, -1, pClassStyle, &pInlineStyle);
      }
   }

   if (!!pInlineStyle)
   {
      pGUIElement->SetStyle(pInlineStyle);
   }
   else
   {
      pGUIElement->SetStyle(pClassStyle);
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
