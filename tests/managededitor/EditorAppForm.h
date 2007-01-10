///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORAPPFORM_H
#define INCLUDED_EDITORAPPFORM_H

#include "EditorDocument.h"
#include "EditorForm.h"
#include "EditorTools.h"
#include "ToolPalette.h"

#include "tech/comtools.h"

#include <vcclr.h>

#ifdef _MSC_VER
#pragma once
#endif

namespace ManagedEditor
{
   class EditorAppFormEntityManagerListener;

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorRenderControl
   //

   ref class EditorRenderControl sealed : public System::Windows::Forms::Control
   {
   public:
      EditorRenderControl();
      ~EditorRenderControl();

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

      void OnHandleCreated(System::Object ^ sender, System::EventArgs ^ e);
      void OnHandleDestroyed(System::Object ^ sender, System::EventArgs ^ e);
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorAppForm
   //

   ref class EditorAppForm sealed : public EditorForm
   {
   public:
      EditorAppForm();
      ~EditorAppForm();
      !EditorAppForm();

      void OnIdle(System::Object ^ sender, System::EventArgs ^ e);
      void OnExit(System::Object ^ sender, System::EventArgs ^ e);

      void OnEntitySelectionChange();

   protected:
      void OnDocumentChange(System::Object ^ sender, DocumentChangeEventArgs ^ e);

      void OnUndo(System::Object ^ sender, System::EventArgs ^ e);
      void OnRedo(System::Object ^ sender, System::EventArgs ^ e);

      void InvokeToolMethod(System::String ^ methodName, array<System::Type ^> ^ paramTypes, array<System::Object ^> ^ params);

      void renderControl_OnResize(System::Object ^ sender, System::EventArgs ^ e);
      void renderControl_OnMouseDown(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void renderControl_OnMouseUp(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void renderControl_OnMouseClick(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void renderControl_OnMouseMove(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void renderControl_OnMouseHover(System::Object ^ sender, System::EventArgs ^ e);
      void renderControl_OnMouseWheel(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);

      void toolPalette_OnToolSelect(System::Object ^ sender, ToolSelectEventArgs ^ e);

      virtual void NewDocument() override;
      virtual void OpenDocument() override;
      virtual void SaveDocument() override;
      virtual void SaveDocumentAs() override;

      void CreateEditorTools();

   private:
      System::Resources::ResourceManager ^ m_resMgr;

      EditorRenderControl ^ m_renderControl;
      ToolPalette ^ m_toolPalette;
      System::Windows::Forms::PropertyGrid ^ m_propertyGrid;

      EditorDocument ^ m_document;

      System::String ^ m_originalUndoText;
      System::String ^ m_originalRedoText;

      EditorAppFormEntityManagerListener * m_pEntityManagerListener;
   };

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorAppFormEntityManagerListener
   //

   class EditorAppFormEntityManagerListener : public cComObject<IMPLEMENTS(IEntityManagerListener)>
   {
   public:
      EditorAppFormEntityManagerListener();

      void SetEditorAppForm(EditorAppForm ^ editorAppForm);

      virtual void OnEntitySelectionChange();

   private:
      gcroot<EditorAppForm ^> m_editorAppForm;
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORAPPFORM_H
