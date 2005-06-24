///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIRENDER_H
#define INCLUDED_GUIRENDER_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIElementRenderer
//

template <class T>
class cGUIElementRenderer : public IGUIElementRenderer
{
public:
   cGUIElementRenderer();
   ~cGUIElementRenderer();

   virtual tResult Render(IGUIElement * pElement);
   virtual tGUISize GetPreferredSize(IGUIElement * pElement);
   virtual tResult GetFont(IGUIElement * pElement, IGUIFont * * ppFont);

protected:
   tResult Render(IGUIButtonElement * pButtonElement);
   tResult Render(IGUIDialogElement * pDialogElement);
   tResult Render(IGUILabelElement * pLabelElement);
   tResult Render(IGUIPanelElement * pPanelElement);
   tResult Render(IGUITextEditElement * pTextEditElement);

   tGUISize GetPreferredSize(IGUIButtonElement * pButtonElement);
   tGUISize GetPreferredSize(IGUIDialogElement * pDialogElement);
   tGUISize GetPreferredSize(IGUILabelElement * pLabelElement);
   tGUISize GetPreferredSize(IGUIPanelElement * pPanelElement);
   tGUISize GetPreferredSize(IGUITextEditElement * pTextEditElement);
   tGUISize GetPreferredSize(IGUIContainerElement * pContainerElement);

   IGUIFont * AccessDefaultFont();

private:
   cAutoIPtr<IGUIFont> m_pDefaultFont;
};

///////////////////////////////////////

template <class T>
cGUIElementRenderer<T>::cGUIElementRenderer()
{
}

///////////////////////////////////////

template <class T>
cGUIElementRenderer<T>::~cGUIElementRenderer()
{
}

///////////////////////////////////////

template <class T>
tResult cGUIElementRenderer<T>::Render(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   T * pT = static_cast<T*>(this);

   {
      cAutoIPtr<IGUIButtonElement> pButtonElement;
      if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButtonElement) == S_OK)
      {
         return pT->Render(pButtonElement);
      }
   }

   {
      cAutoIPtr<IGUIDialogElement> pDialogElement;
      if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialogElement) == S_OK)
      {
         return pT->Render(pDialogElement);
      }
   }

   {
      cAutoIPtr<IGUILabelElement> pLabelElement;
      if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabelElement) == S_OK)
      {
         return pT->Render(pLabelElement);
      }
   }

   {
      cAutoIPtr<IGUIPanelElement> pPanelElement;
      if (pElement->QueryInterface(IID_IGUIPanelElement, (void**)&pPanelElement) == S_OK)
      {
         return pT->Render(pPanelElement);
      }
   }

   {
      cAutoIPtr<IGUITextEditElement> pTextEditElement;
      if (pElement->QueryInterface(IID_IGUITextEditElement, (void**)&pTextEditElement) == S_OK)
      {
         return pT->Render(pTextEditElement);
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

template <class T>
tGUISize cGUIElementRenderer<T>::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      T * pT = static_cast<T*>(this);

      {
         cAutoIPtr<IGUIButtonElement> pButtonElement;
         if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButtonElement) == S_OK)
         {
            return pT->GetPreferredSize(pButtonElement);
         }
      }

      {
         cAutoIPtr<IGUIDialogElement> pDialogElement;
         if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialogElement) == S_OK)
         {
            return pT->GetPreferredSize(pDialogElement);
         }
      }

      {
         cAutoIPtr<IGUILabelElement> pLabelElement;
         if (pElement->QueryInterface(IID_IGUILabelElement, (void**)&pLabelElement) == S_OK)
         {
            return pT->GetPreferredSize(pLabelElement);
         }
      }

      {
         cAutoIPtr<IGUIPanelElement> pPanelElement;
         if (pElement->QueryInterface(IID_IGUIPanelElement, (void**)&pPanelElement) == S_OK)
         {
            return pT->GetPreferredSize(pPanelElement);
         }
      }

      {
         cAutoIPtr<IGUITextEditElement> pTextEditElement;
         if (pElement->QueryInterface(IID_IGUITextEditElement, (void**)&pTextEditElement) == S_OK)
         {
            return pT->GetPreferredSize(pTextEditElement);
         }
      }
   }

   return tGUISize(0,0);
}

///////////////////////////////////////

template <class T>
tResult cGUIElementRenderer<T>::Render(IGUIButtonElement * pButtonElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

template <class T>
tResult cGUIElementRenderer<T>::Render(IGUIDialogElement * pDialogElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

template <class T>
tResult cGUIElementRenderer<T>::Render(IGUILabelElement * pLabelElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

template <class T>
tResult cGUIElementRenderer<T>::Render(IGUIPanelElement * pPanelElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

template <class T>
tResult cGUIElementRenderer<T>::Render(IGUITextEditElement * pTextEditElement)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

template <class T>
tGUISize cGUIElementRenderer<T>::GetPreferredSize(IGUIButtonElement * pButtonElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

template <class T>
tGUISize cGUIElementRenderer<T>::GetPreferredSize(IGUIDialogElement * pDialogElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

template <class T>
tGUISize cGUIElementRenderer<T>::GetPreferredSize(IGUILabelElement * pLabelElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

template <class T>
tGUISize cGUIElementRenderer<T>::GetPreferredSize(IGUIPanelElement * pPanelElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

template <class T>
tGUISize cGUIElementRenderer<T>::GetPreferredSize(IGUITextEditElement * pTextEditElement)
{
   return tGUISize(0,0);
}

///////////////////////////////////////

template <class T>
tGUISize cGUIElementRenderer<T>::GetPreferredSize(IGUIContainerElement * pContainerElement)
{
   cAutoIPtr<IGUILayoutManager> pLayout;
   if (pContainerElement->GetLayout(&pLayout) == S_OK)
   {
      tGUISize size;
      if (pLayout->GetPreferredSize(pContainerElement, &size) == S_OK)
      {
         return size;
      }
   }
   return tGUISize(0,0);
}

///////////////////////////////////////

template <class T>
tResult cGUIElementRenderer<T>::GetFont(IGUIElement * pElement,
                                        IGUIFont * * ppFont)
{
   if (pElement == NULL || ppFont == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      if (pStyle->GetFont(ppFont) == S_OK)
      {
         return S_OK;
      }
   }

   *ppFont = CTAddRef(AccessDefaultFont());
   return S_OK;
}

///////////////////////////////////////

template <class T>
IGUIFont * cGUIElementRenderer<T>::AccessDefaultFont()
{
   if (!m_pDefaultFont)
   {
      GUIFontGetDefault(&m_pDefaultFont);
   }
   return m_pDefaultFont;
}

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_GUIRENDER_H
