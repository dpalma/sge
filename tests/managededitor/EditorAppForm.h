///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORAPPFORM_H
#define INCLUDED_EDITORAPPFORM_H

#include "EditorDocument.h"
#include "EditorForm.h"
#include "EditorTools.h"
#include "GlControl.h"
#include "ToolPalette.h"

#ifdef _MSC_VER
#pragma once
#endif

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

   protected:
      void OnDocumentChange(System::Object ^ sender, DocumentChangeEventArgs ^ e);

      void OnUndo(System::Object ^ sender, System::EventArgs ^ e);
      void OnRedo(System::Object ^ sender, System::EventArgs ^ e);

      void InvokeToolMethod(System::String ^ methodName, array<System::Type ^> ^ paramTypes, array<System::Object ^> ^ params);

      void glControl_OnResize(System::Object ^ sender, System::EventArgs ^ e);
      void glControl_OnMouseDown(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void glControl_OnMouseUp(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void glControl_OnMouseClick(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void glControl_OnMouseMove(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void glControl_OnMouseHover(System::Object ^ sender, System::EventArgs ^ e);
      void glControl_OnMouseWheel(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);

      void toolPalette_OnToolSelect(System::Object ^ sender, ToolSelectEventArgs ^ e);

      virtual void NewDocument() override;
      virtual void OpenDocument() override;
      virtual void SaveDocument() override;
      virtual void SaveDocumentAs() override;

      void CreateEditorTools();

   private:
      System::Resources::ResourceManager ^ m_resMgr;

      GlControl ^ m_glControl;
      ToolPalette ^ m_toolPalette;
      System::Windows::Forms::PropertyGrid ^ m_propertyGrid;

      EditorDocument ^ m_document;

      System::String ^ m_originalUndoText;
      System::String ^ m_originalRedoText;
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORAPPFORM_H
