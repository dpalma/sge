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

namespace ManagedEditor
{

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: EditorForm
//

ref class EditorForm sealed : public System::Windows::Forms::Form
{
public:
   EditorForm();
   ~EditorForm();

   void OnIdle(System::Object ^ sender, System::EventArgs ^ e);

private:
   IGUIFont * m_pFont;

   GlControl ^ m_glControl;

   EditorDocument ^ m_document;

private: System::Windows::Forms::MenuStrip^  editorFormMenuStrip;




private: System::Windows::Forms::ToolStrip^  editorFormToolStrip;
private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem1;
private: System::Windows::Forms::ToolStripMenuItem^  newToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  openToolStripMenuItem1;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator3;
private: System::Windows::Forms::ToolStripMenuItem^  saveToolStripMenuItem1;
private: System::Windows::Forms::ToolStripMenuItem^  saveAsToolStripMenuItem1;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator4;



private: System::Windows::Forms::ToolStripMenuItem^  exitToolStripMenuItem1;
private: System::Windows::Forms::ToolStripMenuItem^  editToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  undoToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  redoToolStripMenuItem;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator6;
private: System::Windows::Forms::ToolStripMenuItem^  cutToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  copyToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  pasteToolStripMenuItem;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator7;
private: System::Windows::Forms::ToolStripMenuItem^  selectAllToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  toolsToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  customizeToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  optionsToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  helpToolStripMenuItem1;
private: System::Windows::Forms::ToolStripMenuItem^  contentsToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  indexToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  searchToolStripMenuItem;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator8;
private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem1;
private: System::Windows::Forms::ToolStripButton^  newToolStripButton;
private: System::Windows::Forms::ToolStripButton^  openToolStripButton;
private: System::Windows::Forms::ToolStripButton^  saveToolStripButton;

private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator;
private: System::Windows::Forms::ToolStripButton^  cutToolStripButton;
private: System::Windows::Forms::ToolStripButton^  copyToolStripButton;
private: System::Windows::Forms::ToolStripButton^  pasteToolStripButton;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
private: System::Windows::Forms::ToolStripButton^  helpToolStripButton;
private: System::Windows::Forms::ToolStripContainer^  toolStripContainer1;

   /// <summary>
   /// Required designer variable.
   /// </summary>
   System::ComponentModel::Container ^ components;

#pragma region Windows Form Designer generated code
   /// <summary>
   /// Required method for Designer support - do not modify
   /// the contents of this method with the code editor.
   /// </summary>
   void EditorForm::InitializeComponent()
   {
	   System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(EditorForm::typeid));
	   this->editorFormMenuStrip = (gcnew System::Windows::Forms::MenuStrip());
	   this->fileToolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->newToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->openToolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->toolStripSeparator3 = (gcnew System::Windows::Forms::ToolStripSeparator());
	   this->saveToolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->saveAsToolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->toolStripSeparator4 = (gcnew System::Windows::Forms::ToolStripSeparator());
	   this->exitToolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->editToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->undoToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->redoToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->toolStripSeparator6 = (gcnew System::Windows::Forms::ToolStripSeparator());
	   this->cutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->copyToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->pasteToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->toolStripSeparator7 = (gcnew System::Windows::Forms::ToolStripSeparator());
	   this->selectAllToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->toolsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->customizeToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->optionsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->helpToolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->contentsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->indexToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->searchToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->toolStripSeparator8 = (gcnew System::Windows::Forms::ToolStripSeparator());
	   this->aboutToolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
	   this->editorFormToolStrip = (gcnew System::Windows::Forms::ToolStrip());
	   this->newToolStripButton = (gcnew System::Windows::Forms::ToolStripButton());
	   this->openToolStripButton = (gcnew System::Windows::Forms::ToolStripButton());
	   this->saveToolStripButton = (gcnew System::Windows::Forms::ToolStripButton());
	   this->toolStripSeparator = (gcnew System::Windows::Forms::ToolStripSeparator());
	   this->cutToolStripButton = (gcnew System::Windows::Forms::ToolStripButton());
	   this->copyToolStripButton = (gcnew System::Windows::Forms::ToolStripButton());
	   this->pasteToolStripButton = (gcnew System::Windows::Forms::ToolStripButton());
	   this->toolStripSeparator2 = (gcnew System::Windows::Forms::ToolStripSeparator());
	   this->helpToolStripButton = (gcnew System::Windows::Forms::ToolStripButton());
	   this->toolStripContainer1 = (gcnew System::Windows::Forms::ToolStripContainer());
	   this->editorFormMenuStrip->SuspendLayout();
	   this->editorFormToolStrip->SuspendLayout();
	   this->toolStripContainer1->TopToolStripPanel->SuspendLayout();
	   this->toolStripContainer1->SuspendLayout();
	   this->SuspendLayout();
	   // 
	   // editorFormMenuStrip
	   // 
	   this->editorFormMenuStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->fileToolStripMenuItem1, 
		   this->editToolStripMenuItem, this->toolsToolStripMenuItem, this->helpToolStripMenuItem1});
	   this->editorFormMenuStrip->Location = System::Drawing::Point(0, 0);
	   this->editorFormMenuStrip->Name = L"editorFormMenuStrip";
	   this->editorFormMenuStrip->Size = System::Drawing::Size(292, 24);
	   this->editorFormMenuStrip->TabIndex = 0;
	   this->editorFormMenuStrip->Text = L"menuStrip1";
	   // 
	   // fileToolStripMenuItem1
	   // 
	   this->fileToolStripMenuItem1->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(7) {this->newToolStripMenuItem, 
		   this->openToolStripMenuItem1, this->toolStripSeparator3, this->saveToolStripMenuItem1, this->saveAsToolStripMenuItem1, this->toolStripSeparator4, 
		   this->exitToolStripMenuItem1});
	   this->fileToolStripMenuItem1->Name = L"fileToolStripMenuItem1";
	   this->fileToolStripMenuItem1->Size = System::Drawing::Size(35, 20);
	   this->fileToolStripMenuItem1->Text = L"&File";
	   // 
	   // newToolStripMenuItem
	   // 
	   this->newToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"newToolStripMenuItem.Image")));
	   this->newToolStripMenuItem->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->newToolStripMenuItem->Name = L"newToolStripMenuItem";
	   this->newToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::N));
	   this->newToolStripMenuItem->Size = System::Drawing::Size(151, 22);
	   this->newToolStripMenuItem->Text = L"&New";
	   this->newToolStripMenuItem->Click += gcnew System::EventHandler(this, &EditorForm::OnFileNew);
	   // 
	   // openToolStripMenuItem1
	   // 
	   this->openToolStripMenuItem1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"openToolStripMenuItem1.Image")));
	   this->openToolStripMenuItem1->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->openToolStripMenuItem1->Name = L"openToolStripMenuItem1";
	   this->openToolStripMenuItem1->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::O));
	   this->openToolStripMenuItem1->Size = System::Drawing::Size(151, 22);
	   this->openToolStripMenuItem1->Text = L"&Open";
	   this->openToolStripMenuItem1->Click += gcnew System::EventHandler(this, &EditorForm::OnFileOpen);
	   // 
	   // toolStripSeparator3
	   // 
	   this->toolStripSeparator3->Name = L"toolStripSeparator3";
	   this->toolStripSeparator3->Size = System::Drawing::Size(148, 6);
	   // 
	   // saveToolStripMenuItem1
	   // 
	   this->saveToolStripMenuItem1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"saveToolStripMenuItem1.Image")));
	   this->saveToolStripMenuItem1->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->saveToolStripMenuItem1->Name = L"saveToolStripMenuItem1";
	   this->saveToolStripMenuItem1->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::S));
	   this->saveToolStripMenuItem1->Size = System::Drawing::Size(151, 22);
	   this->saveToolStripMenuItem1->Text = L"&Save";
	   this->saveToolStripMenuItem1->Click += gcnew System::EventHandler(this, &EditorForm::OnFileSave);
	   // 
	   // saveAsToolStripMenuItem1
	   // 
	   this->saveAsToolStripMenuItem1->Name = L"saveAsToolStripMenuItem1";
	   this->saveAsToolStripMenuItem1->Size = System::Drawing::Size(151, 22);
	   this->saveAsToolStripMenuItem1->Text = L"Save &As";
	   this->saveAsToolStripMenuItem1->Click += gcnew System::EventHandler(this, &EditorForm::OnFileSaveAs);
	   // 
	   // toolStripSeparator4
	   // 
	   this->toolStripSeparator4->Name = L"toolStripSeparator4";
	   this->toolStripSeparator4->Size = System::Drawing::Size(148, 6);
	   // 
	   // exitToolStripMenuItem1
	   // 
	   this->exitToolStripMenuItem1->Name = L"exitToolStripMenuItem1";
	   this->exitToolStripMenuItem1->Size = System::Drawing::Size(151, 22);
	   this->exitToolStripMenuItem1->Text = L"E&xit";
	   this->exitToolStripMenuItem1->Click += gcnew System::EventHandler(this, &EditorForm::OnFileExit);
	   // 
	   // editToolStripMenuItem
	   // 
	   this->editToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(8) {this->undoToolStripMenuItem, 
		   this->redoToolStripMenuItem, this->toolStripSeparator6, this->cutToolStripMenuItem, this->copyToolStripMenuItem, this->pasteToolStripMenuItem, 
		   this->toolStripSeparator7, this->selectAllToolStripMenuItem});
	   this->editToolStripMenuItem->Name = L"editToolStripMenuItem";
	   this->editToolStripMenuItem->Size = System::Drawing::Size(37, 20);
	   this->editToolStripMenuItem->Text = L"&Edit";
	   // 
	   // undoToolStripMenuItem
	   // 
	   this->undoToolStripMenuItem->Name = L"undoToolStripMenuItem";
	   this->undoToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::Z));
	   this->undoToolStripMenuItem->Size = System::Drawing::Size(150, 22);
	   this->undoToolStripMenuItem->Text = L"&Undo";
	   // 
	   // redoToolStripMenuItem
	   // 
	   this->redoToolStripMenuItem->Name = L"redoToolStripMenuItem";
	   this->redoToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::Y));
	   this->redoToolStripMenuItem->Size = System::Drawing::Size(150, 22);
	   this->redoToolStripMenuItem->Text = L"&Redo";
	   // 
	   // toolStripSeparator6
	   // 
	   this->toolStripSeparator6->Name = L"toolStripSeparator6";
	   this->toolStripSeparator6->Size = System::Drawing::Size(147, 6);
	   // 
	   // cutToolStripMenuItem
	   // 
	   this->cutToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"cutToolStripMenuItem.Image")));
	   this->cutToolStripMenuItem->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->cutToolStripMenuItem->Name = L"cutToolStripMenuItem";
	   this->cutToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::X));
	   this->cutToolStripMenuItem->Size = System::Drawing::Size(150, 22);
	   this->cutToolStripMenuItem->Text = L"Cu&t";
	   // 
	   // copyToolStripMenuItem
	   // 
	   this->copyToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"copyToolStripMenuItem.Image")));
	   this->copyToolStripMenuItem->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->copyToolStripMenuItem->Name = L"copyToolStripMenuItem";
	   this->copyToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::C));
	   this->copyToolStripMenuItem->Size = System::Drawing::Size(150, 22);
	   this->copyToolStripMenuItem->Text = L"&Copy";
	   // 
	   // pasteToolStripMenuItem
	   // 
	   this->pasteToolStripMenuItem->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pasteToolStripMenuItem.Image")));
	   this->pasteToolStripMenuItem->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->pasteToolStripMenuItem->Name = L"pasteToolStripMenuItem";
	   this->pasteToolStripMenuItem->ShortcutKeys = static_cast<System::Windows::Forms::Keys>((System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::V));
	   this->pasteToolStripMenuItem->Size = System::Drawing::Size(150, 22);
	   this->pasteToolStripMenuItem->Text = L"&Paste";
	   // 
	   // toolStripSeparator7
	   // 
	   this->toolStripSeparator7->Name = L"toolStripSeparator7";
	   this->toolStripSeparator7->Size = System::Drawing::Size(147, 6);
	   // 
	   // selectAllToolStripMenuItem
	   // 
	   this->selectAllToolStripMenuItem->Name = L"selectAllToolStripMenuItem";
	   this->selectAllToolStripMenuItem->Size = System::Drawing::Size(150, 22);
	   this->selectAllToolStripMenuItem->Text = L"Select &All";
	   // 
	   // toolsToolStripMenuItem
	   // 
	   this->toolsToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->customizeToolStripMenuItem, 
		   this->optionsToolStripMenuItem});
	   this->toolsToolStripMenuItem->Name = L"toolsToolStripMenuItem";
	   this->toolsToolStripMenuItem->Size = System::Drawing::Size(44, 20);
	   this->toolsToolStripMenuItem->Text = L"&Tools";
	   // 
	   // customizeToolStripMenuItem
	   // 
	   this->customizeToolStripMenuItem->Name = L"customizeToolStripMenuItem";
	   this->customizeToolStripMenuItem->Size = System::Drawing::Size(134, 22);
	   this->customizeToolStripMenuItem->Text = L"&Customize";
	   // 
	   // optionsToolStripMenuItem
	   // 
	   this->optionsToolStripMenuItem->Name = L"optionsToolStripMenuItem";
	   this->optionsToolStripMenuItem->Size = System::Drawing::Size(134, 22);
	   this->optionsToolStripMenuItem->Text = L"&Options";
	   // 
	   // helpToolStripMenuItem1
	   // 
	   this->helpToolStripMenuItem1->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {this->contentsToolStripMenuItem, 
		   this->indexToolStripMenuItem, this->searchToolStripMenuItem, this->toolStripSeparator8, this->aboutToolStripMenuItem1});
	   this->helpToolStripMenuItem1->Name = L"helpToolStripMenuItem1";
	   this->helpToolStripMenuItem1->Size = System::Drawing::Size(40, 20);
	   this->helpToolStripMenuItem1->Text = L"&Help";
	   // 
	   // contentsToolStripMenuItem
	   // 
	   this->contentsToolStripMenuItem->Name = L"contentsToolStripMenuItem";
	   this->contentsToolStripMenuItem->Size = System::Drawing::Size(129, 22);
	   this->contentsToolStripMenuItem->Text = L"&Contents";
	   // 
	   // indexToolStripMenuItem
	   // 
	   this->indexToolStripMenuItem->Name = L"indexToolStripMenuItem";
	   this->indexToolStripMenuItem->Size = System::Drawing::Size(129, 22);
	   this->indexToolStripMenuItem->Text = L"&Index";
	   // 
	   // searchToolStripMenuItem
	   // 
	   this->searchToolStripMenuItem->Name = L"searchToolStripMenuItem";
	   this->searchToolStripMenuItem->Size = System::Drawing::Size(129, 22);
	   this->searchToolStripMenuItem->Text = L"&Search";
	   // 
	   // toolStripSeparator8
	   // 
	   this->toolStripSeparator8->Name = L"toolStripSeparator8";
	   this->toolStripSeparator8->Size = System::Drawing::Size(126, 6);
	   // 
	   // aboutToolStripMenuItem1
	   // 
	   this->aboutToolStripMenuItem1->Name = L"aboutToolStripMenuItem1";
	   this->aboutToolStripMenuItem1->Size = System::Drawing::Size(129, 22);
	   this->aboutToolStripMenuItem1->Text = L"&About...";
	   this->aboutToolStripMenuItem1->Click += gcnew System::EventHandler(this, &EditorForm::OnHelpAbout);
	   // 
	   // editorFormToolStrip
	   // 
	   this->editorFormToolStrip->Dock = System::Windows::Forms::DockStyle::None;
	   this->editorFormToolStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(9) {this->newToolStripButton, 
		   this->openToolStripButton, this->saveToolStripButton, this->toolStripSeparator, this->cutToolStripButton, this->copyToolStripButton, 
		   this->pasteToolStripButton, this->toolStripSeparator2, this->helpToolStripButton});
	   this->editorFormToolStrip->Location = System::Drawing::Point(3, 0);
	   this->editorFormToolStrip->Name = L"editorFormToolStrip";
	   this->editorFormToolStrip->Size = System::Drawing::Size(183, 25);
	   this->editorFormToolStrip->TabIndex = 1;
	   this->editorFormToolStrip->Text = L"toolStrip1";
	   // 
	   // newToolStripButton
	   // 
	   this->newToolStripButton->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	   this->newToolStripButton->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"newToolStripButton.Image")));
	   this->newToolStripButton->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->newToolStripButton->Name = L"newToolStripButton";
	   this->newToolStripButton->Size = System::Drawing::Size(23, 22);
	   this->newToolStripButton->Text = L"&New";
	   this->newToolStripButton->Click += gcnew System::EventHandler(this, &EditorForm::OnFileNew);
	   // 
	   // openToolStripButton
	   // 
	   this->openToolStripButton->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	   this->openToolStripButton->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"openToolStripButton.Image")));
	   this->openToolStripButton->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->openToolStripButton->Name = L"openToolStripButton";
	   this->openToolStripButton->Size = System::Drawing::Size(23, 22);
	   this->openToolStripButton->Text = L"&Open";
	   this->openToolStripButton->Click += gcnew System::EventHandler(this, &EditorForm::OnFileOpen);
	   // 
	   // saveToolStripButton
	   // 
	   this->saveToolStripButton->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	   this->saveToolStripButton->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"saveToolStripButton.Image")));
	   this->saveToolStripButton->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->saveToolStripButton->Name = L"saveToolStripButton";
	   this->saveToolStripButton->Size = System::Drawing::Size(23, 22);
	   this->saveToolStripButton->Text = L"&Save";
	   this->saveToolStripButton->Click += gcnew System::EventHandler(this, &EditorForm::OnFileSave);
	   // 
	   // toolStripSeparator
	   // 
	   this->toolStripSeparator->Name = L"toolStripSeparator";
	   this->toolStripSeparator->Size = System::Drawing::Size(6, 25);
	   // 
	   // cutToolStripButton
	   // 
	   this->cutToolStripButton->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	   this->cutToolStripButton->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"cutToolStripButton.Image")));
	   this->cutToolStripButton->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->cutToolStripButton->Name = L"cutToolStripButton";
	   this->cutToolStripButton->Size = System::Drawing::Size(23, 22);
	   this->cutToolStripButton->Text = L"C&ut";
	   // 
	   // copyToolStripButton
	   // 
	   this->copyToolStripButton->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	   this->copyToolStripButton->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"copyToolStripButton.Image")));
	   this->copyToolStripButton->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->copyToolStripButton->Name = L"copyToolStripButton";
	   this->copyToolStripButton->Size = System::Drawing::Size(23, 22);
	   this->copyToolStripButton->Text = L"&Copy";
	   // 
	   // pasteToolStripButton
	   // 
	   this->pasteToolStripButton->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	   this->pasteToolStripButton->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pasteToolStripButton.Image")));
	   this->pasteToolStripButton->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->pasteToolStripButton->Name = L"pasteToolStripButton";
	   this->pasteToolStripButton->Size = System::Drawing::Size(23, 22);
	   this->pasteToolStripButton->Text = L"&Paste";
	   // 
	   // toolStripSeparator2
	   // 
	   this->toolStripSeparator2->Name = L"toolStripSeparator2";
	   this->toolStripSeparator2->Size = System::Drawing::Size(6, 25);
	   // 
	   // helpToolStripButton
	   // 
	   this->helpToolStripButton->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
	   this->helpToolStripButton->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"helpToolStripButton.Image")));
	   this->helpToolStripButton->ImageTransparentColor = System::Drawing::Color::Magenta;
	   this->helpToolStripButton->Name = L"helpToolStripButton";
	   this->helpToolStripButton->Size = System::Drawing::Size(23, 22);
	   this->helpToolStripButton->Text = L"He&lp";
	   this->helpToolStripButton->Click += gcnew System::EventHandler(this, &EditorForm::OnHelpAbout);
	   // 
	   // toolStripContainer1
	   // 
	   // 
	   // toolStripContainer1.ContentPanel
	   // 
	   this->toolStripContainer1->ContentPanel->Size = System::Drawing::Size(292, 224);
	   this->toolStripContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
	   this->toolStripContainer1->Location = System::Drawing::Point(0, 24);
	   this->toolStripContainer1->Name = L"toolStripContainer1";
	   this->toolStripContainer1->Size = System::Drawing::Size(292, 249);
	   this->toolStripContainer1->TabIndex = 2;
	   this->toolStripContainer1->Text = L"toolStripContainer1";
	   // 
	   // toolStripContainer1.TopToolStripPanel
	   // 
	   this->toolStripContainer1->TopToolStripPanel->Controls->Add(this->editorFormToolStrip);
	   // 
	   // EditorForm
	   // 
	   this->BackColor = System::Drawing::SystemColors::Window;
	   this->CausesValidation = false;
	   this->ClientSize = System::Drawing::Size(292, 273);
	   this->Controls->Add(this->toolStripContainer1);
	   this->Controls->Add(this->editorFormMenuStrip);
	   this->ForeColor = System::Drawing::SystemColors::Window;
	   this->MainMenuStrip = this->editorFormMenuStrip;
	   this->Name = L"EditorForm";
	   this->StartPosition = System::Windows::Forms::FormStartPosition::WindowsDefaultBounds;
	   this->Text = L"Editor";
	   this->editorFormMenuStrip->ResumeLayout(false);
	   this->editorFormMenuStrip->PerformLayout();
	   this->editorFormToolStrip->ResumeLayout(false);
	   this->editorFormToolStrip->PerformLayout();
	   this->toolStripContainer1->TopToolStripPanel->ResumeLayout(false);
	   this->toolStripContainer1->TopToolStripPanel->PerformLayout();
	   this->toolStripContainer1->ResumeLayout(false);
	   this->toolStripContainer1->PerformLayout();
	   this->ResumeLayout(false);
	   this->PerformLayout();

   }
#pragma endregion


private: System::Void OnFileOpen(System::Object^  sender, System::EventArgs^  e);
private: System::Void OnFileNew(System::Object^  sender, System::EventArgs^  e);
private: System::Void OnFileExit(System::Object^  sender, System::EventArgs^  e);
private: System::Void OnFileSave(System::Object^  sender, System::EventArgs^  e);
private: System::Void OnFileSaveAs(System::Object^  sender, System::EventArgs^  e);
private: System::Void OnHelpAbout(System::Object^  sender, System::EventArgs^  e);
};

} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORFORM_H
