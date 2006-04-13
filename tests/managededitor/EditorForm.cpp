///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "EditorForm.h"
#include "EditorAbout.h"


///////////////////////////////////////////////////////////////////////////////
//
// NAMESPACE: ManagedEditor
//

namespace ManagedEditor
{
   using namespace System::Windows::Forms;

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorForm
   //

   EditorForm::EditorForm()
   {
      InitializeComponent();
   }

   EditorForm::~EditorForm()
   {
      if (components)
      {
         delete components;
         components = nullptr;
      }
   }

   System::Windows::Forms::SplitterPanel ^ EditorForm::ToolPanel::get()
   {
      return splitContainer2->Panel1;
   }

   System::Windows::Forms::SplitterPanel ^ EditorForm::MainPanel::get()
   {
      return splitContainer1->Panel2;
   }

   System::Void EditorForm::OnFileOpen(System::Object ^ sender, System::EventArgs ^ e)
   {
      OpenDocument();
   }

   System::Void EditorForm::OnFileNew(System::Object ^ sender, System::EventArgs ^ e)
   {
      NewDocument();
   }

   System::Void EditorForm::OnFileExit(System::Object ^ sender, System::EventArgs ^ e)
   {
      Application::Exit();
   }

   System::Void EditorForm::OnFileSave(System::Object ^ sender, System::EventArgs ^ e)
   {
      SaveDocument();
   }

   System::Void EditorForm::OnFileSaveAs(System::Object ^ sender, System::EventArgs ^ e)
   {
      try
      {
         SaveDocumentAs();
      }
      catch (Exception ^ ex)
      {
         MessageBox::Show(ex->ToString());
      }
   }

   System::Void EditorForm::OnHelpAbout(System::Object ^ sender, System::EventArgs ^ e)
   {
      EditorAbout ^ aboutBox = gcnew EditorAbout();
      aboutBox->ShowDialog();
   }

} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////
