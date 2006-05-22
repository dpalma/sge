///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guidialog.h"
#include "guielementbasetem.h"
#include "guicontainerbasetem.h"
#include "guielementenum.h"
#include "guielementtools.h"
#include "guistrings.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(GUIDialogEvents);

#define LocalMsg(msg)                  DebugMsgEx(GUIDialogEvents, (msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(GUIDialogEvents, (msg), (a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(GUIDialogEvents, (msg), (a1), (a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUIDialogEvents, (cond), (msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(GUIDialogEvents, (cond), (msg), (a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(GUIDialogEvents, (cond), (msg), (a1), (a2))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIDialogElement
//

///////////////////////////////////////

cGUIDialogElement::cGUIDialogElement()
{
}

///////////////////////////////////////

cGUIDialogElement::~cGUIDialogElement()
{
}

///////////////////////////////////////

tResult cGUIDialogElement::GetTitle(tGUIString * pTitle)
{
   if (!m_pTitleBar)
   {
      return S_FALSE;
   }
   else
   {
      return m_pTitleBar->GetTitle(pTitle);
   }
}

///////////////////////////////////////

tResult cGUIDialogElement::SetTitle(const tGUIChar * pszTitle)
{
   if (pszTitle == NULL)
   {
      SafeRelease(m_pTitleBar);
      return S_OK;
   }
   else
   {
      if (!m_pTitleBar)
      {
         if (GUITitleBarCreate(&m_pTitleBar) == S_OK)
         {
            // SetParent calls AddElement which will add m_pTitleBar
            // to the dialog's list of children
            m_pTitleBar->SetParent(this);
         }
      }
      if (!!m_pTitleBar)
      {
         return m_pTitleBar->SetTitle(pszTitle);
      }
      return E_FAIL;
   }
}


///////////////////////////////////////////////////////////////////////////////

tResult GUIDialogElementCreate(const TiXmlElement * pXmlElement,
                               IGUIElement * pParent, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIDialogElement> pDialog = static_cast<IGUIDialogElement *>(new cGUIDialogElement);
   if (!pDialog)
   {
      return E_OUTOFMEMORY;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), kElementDialog) == 0)
      {
         const char * pszValue = NULL;
         if ((pszValue = pXmlElement->Attribute(kAttribTitle)) != NULL)
         {
            pDialog->SetTitle(pszValue);
         }

         *ppElement = CTAddRef(pDialog);
         return S_OK;
      }
   }
   else
   {
      *ppElement = CTAddRef(pDialog);
      return S_OK;
   }

   return E_FAIL;
}

AUTOREGISTER_GUIELEMENTFACTORYFN(dialog, GUIDialogElementCreate);


///////////////////////////////////////////////////////////////////////////////
