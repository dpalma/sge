///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guielementtools.h"
#include "guistyle.h"

#include "parse.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

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

   if (pXmlElement->Attribute("insets"))
   {
      double insetVals[4];
      if (ParseTuple(pXmlElement->Attribute("insets"), insetVals, _countof(insetVals)) == 4)
      {
         tGUIInsets insets;
         insets.left = insetVals[0];
         insets.top = insetVals[1];
         insets.right = insetVals[2];
         insets.bottom = insetVals[3];
//         pGUIElement->SetInsets(insets);
      }
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
