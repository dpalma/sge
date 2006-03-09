///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORFORM_H
#define INCLUDED_EDITORFORM_H

#include "GlControl.h"
#include "EditorDocument.h"

#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGUIFont);

namespace Editor
{

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorForm
//

ref class cEditorForm sealed : public System::Windows::Forms::Form
{
public:
   cEditorForm();
   ~cEditorForm();

   void OnIdle(System::Object ^ sender, System::EventArgs ^ e);

private:
   IGUIFont * m_pFont;

   cGlControl ^ m_glControl;
private: System::Windows::Forms::MenuStrip^  editorFormMenuStrip;
private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  exitToolStripMenuItem;

   /// <summary>
   /// Required designer variable.
   /// </summary>
   System::ComponentModel::Container ^ components;

#pragma region Windows Form Designer generated code
   /// <summary>
   /// Required method for Designer support - do not modify
   /// the contents of this method with the code editor.
   /// </summary>
   void cEditorForm::InitializeComponent()
   {
      this->editorFormMenuStrip = (gcnew System::Windows::Forms::MenuStrip());
      this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
      this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
      this->editorFormMenuStrip->SuspendLayout();
      this->SuspendLayout();
      // 
      // editorFormMenuStrip
      // 
      this->editorFormMenuStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->fileToolStripMenuItem});
      this->editorFormMenuStrip->Location = System::Drawing::Point(0, 0);
      this->editorFormMenuStrip->Name = L"editorFormMenuStrip";
      this->editorFormMenuStrip->Size = System::Drawing::Size(292, 24);
      this->editorFormMenuStrip->TabIndex = 0;
      this->editorFormMenuStrip->Text = L"menuStrip1";
      // 
      // fileToolStripMenuItem
      // 
      this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->exitToolStripMenuItem});
      this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
      this->fileToolStripMenuItem->Size = System::Drawing::Size(35, 20);
      this->fileToolStripMenuItem->Text = L"&File";
      // 
      // exitToolStripMenuItem
      // 
      this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
      this->exitToolStripMenuItem->Size = System::Drawing::Size(152, 22);
      this->exitToolStripMenuItem->Text = L"E&xit";
      // 
      // cEditorForm
      // 
      this->BackColor = System::Drawing::SystemColors::Window;
      this->CausesValidation = false;
      this->ClientSize = System::Drawing::Size(292, 273);
      this->Controls->Add(this->editorFormMenuStrip);
      this->ForeColor = System::Drawing::SystemColors::Window;
      this->MainMenuStrip = this->editorFormMenuStrip;
      this->Name = L"cEditorForm";
      this->StartPosition = System::Windows::Forms::FormStartPosition::WindowsDefaultBounds;
      this->Text = L"Editor";
      this->editorFormMenuStrip->ResumeLayout(false);
      this->editorFormMenuStrip->PerformLayout();
      this->ResumeLayout(false);
      this->PerformLayout();

   }
#pragma endregion

};

} // namespace Editor

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORFORM_H
