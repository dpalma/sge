///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "gcommon.h"
#include "uiwidgets.h"
#include "uiwidgutilstem.h"
#include "uirender.h"

#include "inputapi.h"

#include "font.h"
#include "render.h"
#include "material.h"
#include "textureapi.h"
#include "image.h"

#include "keys.h"
#include "vec2.h"
#include "resmgr.h"
#include "globalobj.h"
#include "techtime.h"

#include <locale>

#include "dbgalloc.h" // must be last header

static const int g_3dEdge = 2;

static const cUIColor g_hideousErrorColor(0,1,0,1);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIDialog
//

///////////////////////////////////////

cUIDialog::cUIDialog()
{
   SetMargins(
      cUIMargins(
         kDefaultMargin,
         kDefaultMargin + kCaptionHeight + kCaptionGap,
         kDefaultMargin,
         kDefaultMargin));
}

///////////////////////////////////////

cUIDialog::~cUIDialog()
{
}

///////////////////////////////////////

void cUIDialog::Render(IRenderDevice * pRenderDevice)
{
   UIDraw3dRect(GetScreenRect(), 2, AccessStyle()->GetHighlight(), AccessStyle()->GetShadow(), AccessStyle()->GetFace());

   cUIRect captionRect = GetCaptionRect();

   UIDrawGradientRect(captionRect, AccessStyle()->GetCaption(), AccessStyle()->GetCaption(),
      AccessStyle()->GetCaptionGradient(), AccessStyle()->GetCaptionGradient());

   if (!m_title.empty())
   {
      UIDrawText(m_title.c_str(), m_title.length(), &captionRect,
         kDT_Center | kDT_VCenter | kDT_SingleLine, 
         AccessStyle()->AccessFont(), AccessStyle()->GetCaptionText());
   }

   tBase::Render(pRenderDevice);
}

///////////////////////////////////////

bool cUIDialog::OnEvent(const cUIEvent * pEvent)
{
   if ((pEvent->code == kEventKeyDown) && (pEvent->keyCode == kEscape))
   {
      cUIEvent destroyEvent(kEventDestroy, cUIPoint(0,0), 0, this);
      UIBubbleEvent(this, &destroyEvent);
      delete this;
      return true;
   }
   return tBase::OnEvent(pEvent);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUILabel
//

cUILabel::cUILabel()
{
   NoFocus();
}

///////////////////////////////////////

void cUILabel::Render(IRenderDevice * pRenderDevice)
{
   cUIRect rect = GetScreenRect();
   UIDrawText(m_text.c_str(), m_text.length(), &rect,
      kDT_Center | kDT_VCenter | kDT_SingleLine, AccessStyle()->AccessFont(), AccessStyle()->GetText());
}

///////////////////////////////////////

cUISize cUILabel::GetPreferredSize() const
{
   tRect rect(0,0,0,0);
   AccessStyle()->AccessFont()->DrawText(m_text.c_str(), m_text.length(), 
      kDT_CalcRect | kDT_SingleLine, &rect, AccessStyle()->GetText());
   return cUISize(rect.GetWidth(), rect.GetHeight());
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIImage
//

///////////////////////////////////////

cUIImage::cUIImage()
{
   NoFocus();
}

///////////////////////////////////////

cUIImage::~cUIImage()
{
}

///////////////////////////////////////

void cUIImage::Render(IRenderDevice * pRenderDevice)
{
   if (m_pTex != NULL)
   {
      UIDrawTextureRect(GetScreenRect(), m_pTex);
   }
   else
   {
      // draw a hideous color to indicate error
      UIDrawSolidRect(GetScreenRect(), g_hideousErrorColor);
   }
}

///////////////////////////////////////

cUISize cUIImage::GetPreferredSize() const
{
   return m_size;
}

///////////////////////////////////////

bool cUIImage::LoadImage(const char * pszFilename)
{
   UseGlobal(ResourceManager);

   cImage * pImage = ImageLoad(pResourceManager, pszFilename);
   if (pImage != NULL)
   {
      m_size = cUISize(pImage->GetWidth(), pImage->GetHeight());
      delete pImage;
   }

   UseGlobal(TextureManager);
   if (pTextureManager->GetTexture(pszFilename, &m_pTex) == S_OK)
   {
   }

   // return true unconditionally so that this component will
   // be a placeholder at least if the image fails to load
   return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIButton
//

cUIButton::cUIButton()
 : m_bPressed(false),
   m_bMouseOver(false)
{
}

///////////////////////////////////////

void cUIButton::Render(IRenderDevice * pRenderDevice)
{
   cUIRect screenRect = GetScreenRect();

   cUIPoint offset(0,0);

   if (IsMouseOver() && IsPressed())
   {
      UIDraw3dRect(screenRect, g_3dEdge, AccessStyle()->GetShadow(), AccessStyle()->GetHighlight(), AccessStyle()->GetFace());
      offset = cUIPoint(g_3dEdge, g_3dEdge);
   }
   else
   {
      UIDraw3dRect(screenRect, g_3dEdge, AccessStyle()->GetHighlight(), AccessStyle()->GetShadow(), AccessStyle()->GetFace());
   }

   if (!m_text.empty())
   {
      screenRect.left += offset.x;
      screenRect.top += offset.y;
      UIDrawText(m_text.c_str(), m_text.length(), 
         &screenRect,
         kDT_Center | kDT_VCenter | kDT_SingleLine, 
         AccessStyle()->AccessFont(), 
         IsMouseOver() ? AccessStyle()->GetHot() : AccessStyle()->GetText());
   }
}

///////////////////////////////////////

bool cUIButton::OnEvent(const cUIEvent * pEvent)
{
   if (cUIScriptEventHandler<cUIButton>::FilterEvent(pEvent))
      return true;
   if (pEvent->code == kEventMouseDown)
   {
      m_bPressed = true;
   }
   else if (pEvent->code == kEventMouseUp || pEvent->code == kEventBlur)
   {
      m_bPressed = false;
   }
   else if (pEvent->code == kEventMouseEnter)
   {
      m_bMouseOver = true;
   }
   else if (pEvent->code == kEventMouseLeave)
   {
      m_bMouseOver = false;
   }
   return Base::OnEvent(pEvent);
}

///////////////////////////////////////

cUISize cUIButton::GetPreferredSize() const
{
   tRect rect(0,0,0,0);
   AccessStyle()->AccessFont()->DrawText(m_text.c_str(), m_text.length(), 
      kDT_CalcRect | kDT_SingleLine, &rect, AccessStyle()->GetText());
   return cUISize(rect.GetWidth() + (2 * kTextBorderHorz), 
                  rect.GetHeight() + (2 * kTextBorderVert));
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIBitmapButton
//

///////////////////////////////////////

cUIBitmapButton::cUIBitmapButton()
 : m_size(0,0),
   m_bPressed(false),
   m_bMouseOver(false)
{
}

///////////////////////////////////////

cUIBitmapButton::~cUIBitmapButton()
{
}

///////////////////////////////////////

void cUIBitmapButton::Render(IRenderDevice * pRenderDevice)
{
   eUIButtonState buttonState = kBS_Normal;
   if (IsMouseOver())
   {
      buttonState = IsPressed() ? kBS_Pressed : kBS_Hover;
   }

   UseGlobal(UIRenderingTools);
   cAutoIPtr<IIndexBuffer> pIndexBuffer;
   if (pUIRenderingTools->GetBitmapButtonIndexBuffer(&pIndexBuffer) == S_OK)
   {
      cAutoIPtr<IVertexBuffer> pVertexBuffer;
      if (pUIRenderingTools->GetBitmapButtonVertexBuffer(GetScreenRect(), &pVertexBuffer) == S_OK)
      {
         pRenderDevice->Render(kRP_TriangleFan, 
                               m_pMaterial,
                               kNumBitmapButtonIndices, 
                               pIndexBuffer,
                               UIButtonStateVertexStartIndex(buttonState), 
                               pVertexBuffer);
      }
   }
}

///////////////////////////////////////

bool cUIBitmapButton::OnEvent(const cUIEvent * pEvent)
{
   if (pEvent->code == kEventMouseDown)
   {
      m_bPressed = true;
   }
   else if (pEvent->code == kEventMouseUp || pEvent->code == kEventBlur)
   {
      m_bPressed = false;
   }
   else if (pEvent->code == kEventMouseEnter)
   {
      m_bMouseOver = true;
   }
   else if (pEvent->code == kEventMouseLeave)
   {
      m_bMouseOver = false;
   }

   if (cUIScriptEventHandler<cUIBitmapButton>::FilterEvent(pEvent))
      return true;

   return cUIComponent::OnEvent(pEvent);
}

///////////////////////////////////////

cUISize cUIBitmapButton::GetPreferredSize() const
{
   return cUISize(m_size.width, m_size.height / 4);
}

///////////////////////////////////////

bool cUIBitmapButton::SetBitmap(const char * pszName)
{
   UseGlobal(ResourceManager);

   cImage * pImage = ImageLoad(pResourceManager, pszName);
   if (pImage != NULL)
   {
      m_size = cUISize(pImage->GetWidth(), pImage->GetHeight());

      delete pImage;

      UseGlobal(TextureManager);
      cAutoIPtr<ITexture> pTexture;
      if (pTextureManager->GetTexture(pszName, &pTexture) == S_OK)
      {
         m_pMaterial = MaterialCreate();
         if (m_pMaterial != NULL)
         {
            m_pMaterial->SetTexture(0, pTexture);
            return true;
         }
      }
   }

   DebugMsg1("Warning: Error creating button with bitmap \"%s\"\n", pszName);

   return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIEdit
//

///////////////////////////////////////

cUIEdit::cUIEdit()
 : m_size(kDefaultSize),
   m_selection(&m_text),
   m_timeLastBlink(0),
   m_bCursorBlinkOn(true),
   m_bCursorForceOn(false)
{
}

///////////////////////////////////////

cUIEdit::~cUIEdit()
{
}

///////////////////////////////////////

void cUIEdit::Render(IRenderDevice * pRenderDevice)
{
   cUIRect rect = GetScreenRect();

   UIDraw3dRect(rect, g_3dEdge, AccessStyle()->GetShadow(), AccessStyle()->GetFace(), AccessStyle()->GetBackground());

   rect.left += g_3dEdge + kHorzInset;
   rect.top += kVertInset;
   rect.right -= g_3dEdge + kHorzInset;
   rect.bottom -= kVertInset;

   tRect scissor(Round(rect.left), Round(rect.top), Round(rect.right), Round(rect.bottom));
   pRenderDevice->SetScissorRect(&scissor);

   static const float kCursorWidth = 1.f;

   // Determine the width of the text up to the cursor and offset the left edge
   // if necessary so that the cursor is always in view.
   cUISize size = UIMeasureText(m_text.c_str(), m_selection.GetCursorIndex(), AccessStyle()->AccessFont());
   if (size.width >= rect.GetWidth())
   {
      rect.left -= size.width - rect.GetWidth() + kCursorWidth;
   }

   // Don't clip because it's already done above
   UIDrawText(m_text.c_str(), m_text.length(), &rect, kDT_NoClip, AccessStyle()->AccessFont(), AccessStyle()->GetText());

   // Render the cursor if this widget has focus and its blink cycle is 'on'
   if (IsFocussed() && (m_bCursorBlinkOn || m_bCursorForceOn))
   {
      cUIRect cursorRect(
         rect.left + size.width,
         rect.top + kCursorWidth,
         rect.left + size.width + kCursorWidth,
         rect.bottom - kCursorWidth);

      static const cUIColor BLACK(0,0,0,1);

      UIDrawSolidRect(cursorRect, BLACK);
   }

   static const int kCursorBlinkFreq = 2; // blink this many times per second
   static const float kCursorBlinkPeriod = 1.f / kCursorBlinkFreq;

   float time = TimeGetSecs();

   if (m_timeLastBlink == 0)
   {
      m_timeLastBlink = time;
   }
   else if ((time - m_timeLastBlink) > kCursorBlinkPeriod)
   {
      m_bCursorBlinkOn = !m_bCursorBlinkOn;

      m_timeLastBlink = time;
   }

   pRenderDevice->SetScissorRect(NULL);
}

///////////////////////////////////////

bool cUIEdit::OnEvent(const cUIEvent * pEvent)
{
   if (pEvent->code == kEventMouseDown)
   {
      m_selection.SetCursorFromPoint(pEvent->mousePos - GetScreenRect().GetTopLeft());
   }
   else if (pEvent->code == kEventKeyUp)
   {
      m_bCursorForceOn = false;
      m_bCursorBlinkOn = true;
      m_timeLastBlink = TimeGetSecs();
   }
   else if (pEvent->code == kEventKeyDown)
   {
      // attempt to keep the cursor visible if the user is going to
      // hold down any key for a number of repeat characters
      m_bCursorForceOn = 
         (pEvent->keyCode != kCtrl) && 
         (pEvent->keyCode != kLShift) && 
         (pEvent->keyCode != kRShift);

      switch (pEvent->keyCode)
      {
         case kBackspace:
         {
            m_selection.Backspace();
            break;
         }
         case kDelete:
         {
            m_selection.Delete();
            break;
         }
         case kLeft:
         {
            UseGlobal(Input);
            if (pInput->KeyIsDown(kCtrl))
            {
               m_selection.WordLeft();
            }
            else
            {
               m_selection.CharLeft();
            }
            break;
         }
         case kRight:
         {
            UseGlobal(Input);
            if (pInput->KeyIsDown(kCtrl))
            {
               m_selection.WordRight();               
            }
            else
            {
               m_selection.CharRight();
            }
            break;
         }
         case kHome:
         {
            m_selection.Start();
            break;
         }
         case kEnd:
         {
            m_selection.End();
            break;
         }
         case kEscape:
         {
            // don't eat escape keypresses
            return false;
         }
         default:
         {
            if (isprint(pEvent->keyCode))
            {
               m_selection.ReplaceSel((char)pEvent->keyCode);
            }
            break;
         }
      }

      return true;
   }

   return cUIWidget::OnEvent(pEvent);
}

///////////////////////////////////////

cUISize cUIEdit::GetPreferredSize() const
{
   char * psz = reinterpret_cast<char *>(alloca(m_size * sizeof(char)));
   memset(psz, 'M', m_size);

   tRect rect(0,0,0,0);
   AccessStyle()->AccessFont()->DrawText(psz, m_size, 
      kDT_CalcRect | kDT_SingleLine, &rect, AccessStyle()->GetText());

   return cUISize(rect.GetWidth() + (kHorzInset * 2), 
                  rect.GetHeight() + (kVertInset * 2));
}

///////////////////////////////////////////////////////////////////////////////
