///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GLCONTROL_H
#define INCLUDED_GLCONTROL_H

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_HANDLE(HDC);
F_DECLARE_HANDLE(HGLRC);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGlControl
//

ref class cGlControl : public System::Windows::Forms::UserControl
{
public:
   cGlControl();

   void SwapBuffers();

protected:
   virtual property System::Windows::Forms::CreateParams ^ CreateParams
   {
      System::Windows::Forms::CreateParams ^ get() override
      {
         System::Windows::Forms::CreateParams ^ cp = System::Windows::Forms::UserControl::CreateParams;
         // Add style bits required by OpenGL
         cp->Style |= 0x02000000L | 0x04000000L; // WS_CLIPCHILDREN | WS_CLIPSIBLINGS
         return cp;     
      }
  } 
   virtual void OnHandleCreated(System::EventArgs ^ e) override;
   virtual void OnHandleDestroyed(System::EventArgs ^ e) override;

private:
   HDC m_hDc;
   HGLRC m_hGlrc;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GLCONTROL_H
