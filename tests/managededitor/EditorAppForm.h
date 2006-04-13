///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORAPPFORM_H
#define INCLUDED_EDITORAPPFORM_H

#include "EditorDocument.h"
#include "EditorForm.h"
#include "EditorTools.h"
#include "GlControl.h"
#include "ToolPalette.h"

#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGUIFont);

///////////////////////////////////////////////////////////////////////////////
//
// NAMESPACE: ManagedEditor
//

namespace ManagedEditor
{

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorAppForm
   //

   ref class EditorAppForm sealed : public EditorForm
   {
   public:
      EditorAppForm();
      ~EditorAppForm();

      void OnIdle(System::Object ^ sender, System::EventArgs ^ e);

      void OnToolSelect(System::Object ^ sender, ToolSelectEventArgs ^ e);

   protected:
      virtual void NewDocument() override;
      virtual void OpenDocument() override;
      virtual void SaveDocument() override;
      virtual void SaveDocumentAs() override;

   private:
      IGUIFont * m_pFont;

      GlControl ^ m_glControl;

      ToolPalette ^ m_toolPalette;

      EditorDocument ^ m_document;
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORAPPFORM_H
