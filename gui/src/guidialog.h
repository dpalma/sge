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

typedef cGUIContainerBase<IGUIDialogElement> tGUIDialogBase;

class cGUIDialogElement : public cComObject<tGUIDialogBase, &IID_IGUIDialogElement>
{
public:
   cGUIDialogElement();
   ~cGUIDialogElement();

   virtual tResult STDMETHODCALLTYPE QueryInterface(REFGUID iid,
                                                    void * * ppvObject)
   {
      const struct sQIPair pairs[] =
      {
         { static_cast<IGUIDialogElement *>(this), &IID_IGUIDialogElement },
         { static_cast<IGUIContainerElement *>(this), &IID_IGUIContainerElement }
      };
      return DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }

   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual tResult GetTitle(tGUIString * pTitle);
   virtual tResult SetTitle(const char * pszTitle);

   virtual tResult GetCaptionHeight(uint * pHeight);
   virtual tResult SetCaptionHeight(uint height);

private:
   tGUIRect GetCaptionRectAbsolute();

   bool m_bDragging;
   tGUIPoint m_dragOffset;

   tGUIString m_title;
   uint m_captionHeight;
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

#endif // !INCLUDED_GUIDIALOG_H
