#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace ManagedEditor {

	/// <summary>
	/// Summary for EditorAbout
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class EditorAbout : public System::Windows::Forms::Form
	{
	public:
		EditorAbout(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~EditorAbout()
		{
			if (components)
			{
				delete components;
			}
		}
   private: System::Windows::Forms::Button^  button1;
   private: System::Windows::Forms::Button^  okButton;

   private: System::Windows::Forms::Label^  label1;
   private: System::Windows::Forms::Label^  label2;
   protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
         this->button1 = (gcnew System::Windows::Forms::Button());
         this->okButton = (gcnew System::Windows::Forms::Button());
         this->label1 = (gcnew System::Windows::Forms::Label());
         this->label2 = (gcnew System::Windows::Forms::Label());
         this->SuspendLayout();
         // 
         // button1
         // 
         this->button1->Location = System::Drawing::Point(0, 0);
         this->button1->Name = L"button1";
         this->button1->Size = System::Drawing::Size(75, 23);
         this->button1->TabIndex = 0;
         // 
         // okButton
         // 
         this->okButton->DialogResult = System::Windows::Forms::DialogResult::OK;
         this->okButton->Location = System::Drawing::Point(153, 173);
         this->okButton->Name = L"okButton";
         this->okButton->Size = System::Drawing::Size(75, 23);
         this->okButton->TabIndex = 0;
         this->okButton->Text = L"OK";
         this->okButton->UseVisualStyleBackColor = true;
         // 
         // label1
         // 
         this->label1->AutoSize = true;
         this->label1->Location = System::Drawing::Point(12, 21);
         this->label1->Name = L"label1";
         this->label1->Size = System::Drawing::Size(59, 13);
         this->label1->TabIndex = 1;
         this->label1->Text = L"SGE Editor";
         // 
         // label2
         // 
         this->label2->AutoSize = true;
         this->label2->Location = System::Drawing::Point(12, 47);
         this->label2->Name = L"label2";
         this->label2->Size = System::Drawing::Size(161, 13);
         this->label2->TabIndex = 1;
         this->label2->Text = L"Copyright (c) 2006, Daniel Palma";
         // 
         // EditorAbout
         // 
         this->AcceptButton = this->okButton;
         this->ClientSize = System::Drawing::Size(240, 208);
         this->Controls->Add(this->label2);
         this->Controls->Add(this->label1);
         this->Controls->Add(this->okButton);
         this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
         this->MaximizeBox = false;
         this->MinimizeBox = false;
         this->Name = L"EditorAbout";
         this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
         this->Text = L"About Editor";
         this->ResumeLayout(false);
         this->PerformLayout();

      }
#pragma endregion
	};
}
