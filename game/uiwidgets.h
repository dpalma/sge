///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UIWIDGETS_H
#define INCLUDED_UIWIDGETS_H

#include "ui.h"
#include "uiwidgutils.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IUIStyle);
class cUIRect;

F_DECLARE_INTERFACE(IMaterial);
F_DECLARE_INTERFACE(ITexture);
F_DECLARE_INTERFACE(IVertexBuffer);
F_DECLARE_INTERFACE(IIndexBuffer);

///////////////////////////////////////////////////////////////////////////////
// Base class for user interface "widgets"

template <class T>
class cUIWidgetBase : public T
{
protected:
   cUIWidgetBase();

   const IUIStyle * AccessStyle() const;

private:
   cAutoIPtr<IUIStyle> m_pStyle;
};

///////////////////////////////////////

template <class T>
cUIWidgetBase<T>::cUIWidgetBase()
 : m_pStyle(UIStyleCreateDefault())
{
}

///////////////////////////////////////

template <class T>
const IUIStyle * cUIWidgetBase<T>::AccessStyle() const
{
   return (const IUIStyle *)m_pStyle;
}

///////////////////////////////////////

typedef cUIWidgetBase<cUIComponent> cUIWidget;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIDialog
//

class cUIDialog : public cUIWidgetBase<cUIContainer>
{
   typedef cUIContainer tBase;

   enum
   {
      kDefaultMargin = 5,
      kCaptionGap = 3,
      kCaptionHeight = 20,
   };

public:
   cUIDialog();
   ~cUIDialog();

   virtual void Render(IRenderDevice * pRenderDevice);
   virtual bool OnEvent(const cUIEvent * pEvent);

   void SetTitle(const char * pszTitle);

   bool QueryStartDrag(const cUIPoint & mousePos);
   void UpdateDrag(const cUIPoint & mousePos);

   virtual bool OnKeyEvent(long key, bool down, double time);

private:
   cUIRect GetCaptionRect() const;

   cUIString m_title;
   cUIPoint m_dragOffset;
};

///////////////////////////////////////

inline void cUIDialog::SetTitle(const char * pszTitle)
{
   if (pszTitle != NULL)
      m_title = pszTitle;
}

///////////////////////////////////////

inline bool cUIDialog::QueryStartDrag(const cUIPoint & mousePos)
{
   if (GetCaptionRect().PtInside(mousePos))
   {
      m_dragOffset = mousePos - GetPos();
      return true;
   }
   return false;
}

///////////////////////////////////////

inline void cUIDialog::UpdateDrag(const cUIPoint & mousePos)
{
   SetPos(mousePos.x - m_dragOffset.x, mousePos.y - m_dragOffset.y);
}

///////////////////////////////////////

inline cUIRect cUIDialog::GetCaptionRect() const
{
   cUIRect rect = GetScreenRect();

   rect.left += kCaptionGap;
   rect.top += kCaptionGap;
   rect.right -= kCaptionGap;
   rect.bottom = rect.top + kCaptionHeight;

   return rect;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUILabel
//

class cUILabel : public cUIWidget
{
public:
   cUILabel();

   // cUIComponent over-rides
   virtual void Render(IRenderDevice * pRenderDevice);
   virtual cUISize GetPreferredSize() const;

   const char * GetText() const;
   void SetText(const char * psz);

private:
   cUIString m_text;
};

///////////////////////////////////////

inline const char * cUILabel::GetText() const
{
   return m_text.c_str();
}

///////////////////////////////////////

inline void cUILabel::SetText(const char * psz)
{
   Assert(psz != NULL);
   m_text = psz;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIImage
//

class cUIImage : public cUIWidget
{
public:
   cUIImage();
   virtual ~cUIImage();

   virtual void Render(IRenderDevice * pRenderDevice);
   virtual cUISize GetPreferredSize() const;

   bool LoadImage(const char * pszFilename);

private:
   cAutoIPtr<ITexture> m_pTex;
   cUISize m_size;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIButton
//

enum eUIButtonState
{
   kBS_Normal,
   kBS_Hover,
   kBS_Pressed,
   kBS_Disabled
};

class cUIButton
 : public cUIWidget,
   public cUIScriptEventHandler<cUIButton>
{
   typedef cUIWidget Base;

public:
   cUIButton();

   virtual void Render(IRenderDevice * pRenderDevice);
   virtual bool OnEvent(const cUIEvent * pEvent);
   virtual cUISize GetPreferredSize() const;

   const char * GetText() const;
   void SetText(const char * psz);

   void SetOnClick(const char * pszOnClick);

private:
   bool IsPressed();

   enum eUIButtonConstants
   {
      kTextBorderHorz = 10,
      kTextBorderVert = 5,
   };

   cUIString m_text;

   bool m_bPressed;
};

///////////////////////////////////////

inline const char * cUIButton::GetText() const
{
   return m_text.c_str();
}

///////////////////////////////////////

inline void cUIButton::SetText(const char * psz)
{
   Assert(psz != NULL);
   m_text = psz;
}

///////////////////////////////////////

inline void cUIButton::SetOnClick(const char * pszOnClick)
{
   SetEventHandler(kEventClick, pszOnClick);
}

///////////////////////////////////////

inline bool cUIButton::IsPressed()
{
   return m_bPressed;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIBitmapButton
//

class cUIBitmapButton
 : public cUIWidget,
   public cUIScriptEventHandler<cUIBitmapButton>
{
public:
   cUIBitmapButton();
   virtual ~cUIBitmapButton();

   virtual void Render(IRenderDevice * pRenderDevice);
   virtual bool OnEvent(const cUIEvent * pEvent);
   virtual cUISize GetPreferredSize() const;

   bool SetBitmap(const char * pszName);

   void SetOnClick(const char * pszOnClick);

private:
   bool IsPressed();

   enum eConstants
   {
      kNumVerts = 16,
   };

   cUISize m_size;
   cAutoIPtr<IMaterial> m_pMat;
   cAutoIPtr<IVertexBuffer> m_pVB;
   cAutoIPtr<IIndexBuffer> m_pIB;

   bool m_bPressed;
};

///////////////////////////////////////

inline void cUIBitmapButton::SetOnClick(const char * pszOnClick)
{
   SetEventHandler(kEventClick, pszOnClick);
}

///////////////////////////////////////

inline bool cUIBitmapButton::IsPressed()
{
   return m_bPressed;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIEdit
//

class cUIEdit : public cUIWidget
{
   enum
   {
      kVertInset = 2,
      kHorzInset = 2,
      kDefaultSize = 20
   };

public:
   cUIEdit();
   virtual ~cUIEdit();

   virtual void Render(IRenderDevice * pRenderDevice);
   virtual bool OnEvent(const cUIEvent * pEvent);
   virtual cUISize GetPreferredSize() const;

   void SetSize(uint size);

private:
   uint m_size;
   cUIString m_text;
   cUITextSelection m_selection;
   float m_timeLastBlink;
   bool m_bCursorBlinkOn, m_bCursorForceOn;
};

///////////////////////////////////////

inline void cUIEdit::SetSize(uint size)
{
   m_size = size;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_UIWIDGETS_H
