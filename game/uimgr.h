///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIMGR_H
#define INCLUDED_UIMGR_H

#include "ui.h"
#include "uiparse.h"
#include "inputapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIManager
//

class cUIManager : private cUIContainer
{
   typedef cUIContainer tContainerBase;

public:
   cUIManager();
   virtual ~cUIManager();

   ////////////////////////////////////

   void ShowModalDialog(const char * pszXmlFile);
   void Render();

   ////////////////////////////////////
   // cUIContainer/cUIComponent over-rides

   virtual cUISize GetSize() const;
   virtual bool OnEvent(const cUIEvent * pEvent, tUIResult * pResult);

private:
   void PreBubbleEvent(cUIComponent * pEventTarget, cUIEvent * pEvent);

   class cDialogParseHook : public cUIParseHook
   {
   public:
      cDialogParseHook();
      virtual ~cDialogParseHook();
   
      virtual eSkipResult SkipElement(const char * pszElement);

   private:
      int m_nDlgsSeen;
   };

   class cInputListener : public cComObject<IMPLEMENTS(IInputListener)>
   {
      void CDECL operator delete(void *) { Assert(!"Should never be called"); }
   public:
      virtual bool OnMouseEvent(int x, int y, uint mouseState, double time) { return false; }
      virtual bool OnKeyEvent(long key, bool down, double time);
   };

   friend class cInputListener;
   cInputListener m_inputListener;

   cUIComponent * m_pLastMouseOver;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_UIMGR_H
