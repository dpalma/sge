///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIDIALOG_H
#define INCLUDED_GUIDIALOG_H

#include "guielementbase.h"
#include "guicontainerbase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogElement
//

class cGUIDialogElement : public cComObject<cGUIElementBase< cGUIContainerBase<IGUIDialogElement> >, 
                                            &IID_IGUIDialogElement>
{
   typedef cComObject<cGUIElementBase< cGUIContainerBase<IGUIDialogElement> >, 
                      &IID_IGUIDialogElement> tBaseClass;

public:
   cGUIDialogElement();
   ~cGUIDialogElement();

   virtual tResult STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                                    void * * ppvObject)
   {
      const struct sQIPair pairs[] =
      {
         { static_cast<IGUIDialogElement *>(this), &IID_IGUIDialogElement },
         { static_cast<IGUIContainerElement *>(this), &IID_IGUIContainerElement }
      };
      return DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }

   virtual void SetSize(const tGUISize & size);
   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual tResult GetRendererClass(tGUIString * pRendererClass);

   virtual tResult GetInsets(tGUIInsets * pInsets);

   virtual tResult GetTitle(tGUIString * pTitle);
   virtual tResult SetTitle(const char * pszTitle);

   virtual tResult GetCaptionHeight(uint * pHeight);
   virtual tResult SetCaptionHeight(uint height);

   virtual tResult SetModal(bool bModal);
   virtual bool IsModal();

   virtual tResult Accept();
   virtual tResult Cancel();

   virtual tResult GetOnOK(tGUIString * pOnOK) const;
   virtual tResult SetOnOK(const char * pszOnOK);

   virtual tResult GetOnCancel(tGUIString * pOnCancel) const;
   virtual tResult SetOnCancel(const char * pszOnCancel);

private:
   tGUIRect GetCaptionRectAbsolute();

   bool m_bDragging;
   tGUIPoint m_dragOffset;

   tGUIString m_title;
   uint m_captionHeight;
   bool m_bModal;
   tGUIString m_onOK;
   tGUIString m_onCancel;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogElementFactory
//

class cGUIDialogElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogRenderer
//

class cGUIDialogRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUIDialogRenderer();
   ~cGUIDialogRenderer();

   virtual tResult Render(IGUIElement * pElement, IRenderDevice * pRenderDevice);

   virtual tGUISize GetPreferredSize(IGUIElement * pElement);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogRendererFactory
//

class cGUIDialogRendererFactory : public cComObject<IMPLEMENTS(IGUIElementRendererFactory)>
{
public:
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer);
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIDIALOG_H
