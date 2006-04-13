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
// NAMESPACE: ManagedEditor
//

namespace ManagedEditor
{

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: GlControl
   //

   ref class GlControl sealed : public System::Windows::Forms::Control
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
            System::Windows::Forms::CreateParams ^ cp = System::Windows::Forms::Control::CreateParams;
            // Add style bits required by OpenGL
            cp->Style |= 0x06000000L; // WS_CLIPCHILDREN | WS_CLIPSIBLINGS
            return cp;
         }
      }
      virtual void OnHandleCreated(System::EventArgs ^ e) override;
      virtual void OnHandleDestroyed(System::EventArgs ^ e) override;

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
