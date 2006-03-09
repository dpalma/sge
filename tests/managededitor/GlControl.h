///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GLCONTROL_H
#define INCLUDED_GLCONTROL_H

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_HANDLE(HDC);
F_DECLARE_HANDLE(HGLRC);

namespace ManagedEditor
{


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: GlControl
//

ref class GlControl : public System::Windows::Forms::UserControl
{
public:
   GlControl();
   ~GlControl();

   void SwapBuffers();

protected:
   virtual property System::Windows::Forms::CreateParams ^ CreateParams
   {
      System::Windows::Forms::CreateParams ^ get() override
      {
         System::Windows::Forms::CreateParams ^ cp = System::Windows::Forms::UserControl::CreateParams;
         // Add style bits required by OpenGL
         cp->Style |= 0x06000000L; // WS_CLIPCHILDREN | WS_CLIPSIBLINGS
         return cp;
      }
   }
   virtual void OnHandleCreated(System::EventArgs ^ e) override;
   virtual void OnHandleDestroyed(System::EventArgs ^ e) override;

   virtual void OnResize(System::EventArgs ^ e) override;

private:
   void CreateContext();
   void DestroyContext();

private:
   HDC m_hDc;
   HGLRC m_hGlrc;
};


} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GLCONTROL_H
