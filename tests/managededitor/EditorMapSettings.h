///////////////////////////////////////////////////////////////////////////////
// $Id$

#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace ManagedEditor
{
   ref class EditorMapSettings;

   /// <summary>
	/// Summary for EditorMapSettings
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class EditorMapSettingsDlg : public System::Windows::Forms::Form
	{
	public:
		EditorMapSettingsDlg(EditorMapSettings ^ mapSettings)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
         m_mapSettings = mapSettings;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~EditorMapSettingsDlg()
		{
			if (components)
			{
				delete components;
			}
		}
   private: System::Windows::Forms::Button^  okButton;
   private: System::Windows::Forms::Button^  cancelButton;
   protected: 



   private: System::Windows::Forms::ComboBox^  widthComboBox;

   private: System::Windows::Forms::ComboBox^  heightComboBox;

   private: System::Windows::Forms::ComboBox^  tileSetComboBox;

   private: System::Windows::Forms::ComboBox^  initialTileComboBox;

   private: System::Windows::Forms::RadioButton^  uniformHeightRadioButton;
   private: System::Windows::Forms::RadioButton^  genNoiseRadioButton;
   private: System::Windows::Forms::RadioButton^  heightMapRadioButton;
   private: System::Windows::Forms::TextBox^  heightMapTextBox;
   private: System::Windows::Forms::Button^  heightMapBrowseButton;
   private: System::Windows::Forms::NumericUpDown^  heightScaleUpDown;





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
         System::Windows::Forms::Label^  label1;
         System::Windows::Forms::Label^  label2;
         System::Windows::Forms::Label^  label3;
         System::Windows::Forms::Label^  label4;
         System::Windows::Forms::Label^  label6;
         System::Windows::Forms::Label^  label5;
         this->okButton = (gcnew System::Windows::Forms::Button());
         this->cancelButton = (gcnew System::Windows::Forms::Button());
         this->widthComboBox = (gcnew System::Windows::Forms::ComboBox());
         this->heightComboBox = (gcnew System::Windows::Forms::ComboBox());
         this->tileSetComboBox = (gcnew System::Windows::Forms::ComboBox());
         this->initialTileComboBox = (gcnew System::Windows::Forms::ComboBox());
         this->uniformHeightRadioButton = (gcnew System::Windows::Forms::RadioButton());
         this->genNoiseRadioButton = (gcnew System::Windows::Forms::RadioButton());
         this->heightMapRadioButton = (gcnew System::Windows::Forms::RadioButton());
         this->heightMapTextBox = (gcnew System::Windows::Forms::TextBox());
         this->heightMapBrowseButton = (gcnew System::Windows::Forms::Button());
         this->heightScaleUpDown = (gcnew System::Windows::Forms::NumericUpDown());
         label1 = (gcnew System::Windows::Forms::Label());
         label2 = (gcnew System::Windows::Forms::Label());
         label3 = (gcnew System::Windows::Forms::Label());
         label4 = (gcnew System::Windows::Forms::Label());
         label6 = (gcnew System::Windows::Forms::Label());
         label5 = (gcnew System::Windows::Forms::Label());
         (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->heightScaleUpDown))->BeginInit();
         this->SuspendLayout();
         // 
         // label1
         // 
         label1->AutoSize = true;
         label1->Location = System::Drawing::Point(12, 12);
         label1->Name = L"label1";
         label1->Size = System::Drawing::Size(35, 13);
         label1->TabIndex = 2;
         label1->Text = L"Width";
         // 
         // label2
         // 
         label2->AutoSize = true;
         label2->Location = System::Drawing::Point(12, 42);
         label2->Name = L"label2";
         label2->Size = System::Drawing::Size(38, 13);
         label2->TabIndex = 4;
         label2->Text = L"Height";
         // 
         // label3
         // 
         label3->AutoSize = true;
         label3->Location = System::Drawing::Point(12, 72);
         label3->Name = L"label3";
         label3->Size = System::Drawing::Size(43, 13);
         label3->TabIndex = 6;
         label3->Text = L"Tile Set";
         // 
         // label4
         // 
         label4->AutoSize = true;
         label4->Location = System::Drawing::Point(12, 102);
         label4->Name = L"label4";
         label4->Size = System::Drawing::Size(51, 13);
         label4->TabIndex = 8;
         label4->Text = L"Initial Tile";
         // 
         // label6
         // 
         label6->AutoSize = true;
         label6->Location = System::Drawing::Point(7, 234);
         label6->Name = L"label6";
         label6->Size = System::Drawing::Size(68, 13);
         label6->TabIndex = 16;
         label6->Text = L"Height Scale";
         // 
         // label5
         // 
         label5->AutoSize = true;
         label5->Location = System::Drawing::Point(12, 130);
         label5->Name = L"label5";
         label5->Size = System::Drawing::Size(38, 13);
         label5->TabIndex = 10;
         label5->Text = L"Height";
         // 
         // okButton
         // 
         this->okButton->Location = System::Drawing::Point(124, 277);
         this->okButton->Name = L"okButton";
         this->okButton->Size = System::Drawing::Size(75, 23);
         this->okButton->TabIndex = 0;
         this->okButton->Text = L"OK";
         this->okButton->UseVisualStyleBackColor = true;
         // 
         // cancelButton
         // 
         this->cancelButton->DialogResult = System::Windows::Forms::DialogResult::Cancel;
         this->cancelButton->Location = System::Drawing::Point(205, 277);
         this->cancelButton->Name = L"cancelButton";
         this->cancelButton->Size = System::Drawing::Size(75, 23);
         this->cancelButton->TabIndex = 1;
         this->cancelButton->Text = L"Cancel";
         this->cancelButton->UseVisualStyleBackColor = true;
         // 
         // widthComboBox
         // 
         this->widthComboBox->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
         this->widthComboBox->FormattingEnabled = true;
         this->widthComboBox->Items->AddRange(gcnew cli::array< System::Object^  >(4) {L"64", L"128", L"256", L"512"});
         this->widthComboBox->Location = System::Drawing::Point(69, 9);
         this->widthComboBox->Name = L"widthComboBox";
         this->widthComboBox->Size = System::Drawing::Size(121, 21);
         this->widthComboBox->TabIndex = 3;
         this->widthComboBox->Validated += gcnew System::EventHandler(this, &EditorMapSettingsDlg::OnValidated);
         // 
         // heightComboBox
         // 
         this->heightComboBox->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
         this->heightComboBox->FormattingEnabled = true;
         this->heightComboBox->Items->AddRange(gcnew cli::array< System::Object^  >(4) {L"64", L"128", L"256", L"512"});
         this->heightComboBox->Location = System::Drawing::Point(69, 39);
         this->heightComboBox->Name = L"heightComboBox";
         this->heightComboBox->Size = System::Drawing::Size(121, 21);
         this->heightComboBox->TabIndex = 5;
         this->heightComboBox->Validated += gcnew System::EventHandler(this, &EditorMapSettingsDlg::OnValidated);
         // 
         // tileSetComboBox
         // 
         this->tileSetComboBox->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
         this->tileSetComboBox->FormattingEnabled = true;
         this->tileSetComboBox->Location = System::Drawing::Point(69, 69);
         this->tileSetComboBox->Name = L"tileSetComboBox";
         this->tileSetComboBox->Size = System::Drawing::Size(121, 21);
         this->tileSetComboBox->TabIndex = 7;
         // 
         // initialTileComboBox
         // 
         this->initialTileComboBox->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
         this->initialTileComboBox->FormattingEnabled = true;
         this->initialTileComboBox->Location = System::Drawing::Point(69, 99);
         this->initialTileComboBox->Name = L"initialTileComboBox";
         this->initialTileComboBox->Size = System::Drawing::Size(121, 21);
         this->initialTileComboBox->TabIndex = 9;
         // 
         // uniformHeightRadioButton
         // 
         this->uniformHeightRadioButton->AutoSize = true;
         this->uniformHeightRadioButton->Location = System::Drawing::Point(69, 130);
         this->uniformHeightRadioButton->Name = L"uniformHeightRadioButton";
         this->uniformHeightRadioButton->Size = System::Drawing::Size(95, 17);
         this->uniformHeightRadioButton->TabIndex = 11;
         this->uniformHeightRadioButton->TabStop = true;
         this->uniformHeightRadioButton->Text = L"Uniform Height";
         this->uniformHeightRadioButton->UseVisualStyleBackColor = true;
         // 
         // genNoiseRadioButton
         // 
         this->genNoiseRadioButton->AutoSize = true;
         this->genNoiseRadioButton->Location = System::Drawing::Point(69, 153);
         this->genNoiseRadioButton->Name = L"genNoiseRadioButton";
         this->genNoiseRadioButton->Size = System::Drawing::Size(99, 17);
         this->genNoiseRadioButton->TabIndex = 12;
         this->genNoiseRadioButton->TabStop = true;
         this->genNoiseRadioButton->Text = L"Generate Noise";
         this->genNoiseRadioButton->UseVisualStyleBackColor = true;
         // 
         // heightMapRadioButton
         // 
         this->heightMapRadioButton->AutoSize = true;
         this->heightMapRadioButton->Location = System::Drawing::Point(69, 176);
         this->heightMapRadioButton->Name = L"heightMapRadioButton";
         this->heightMapRadioButton->Size = System::Drawing::Size(99, 17);
         this->heightMapRadioButton->TabIndex = 13;
         this->heightMapRadioButton->TabStop = true;
         this->heightMapRadioButton->Text = L"Height Map File";
         this->heightMapRadioButton->UseVisualStyleBackColor = true;
         // 
         // heightMapTextBox
         // 
         this->heightMapTextBox->Location = System::Drawing::Point(81, 199);
         this->heightMapTextBox->Name = L"heightMapTextBox";
         this->heightMapTextBox->Size = System::Drawing::Size(118, 20);
         this->heightMapTextBox->TabIndex = 14;
         // 
         // heightMapBrowseButton
         // 
         this->heightMapBrowseButton->Location = System::Drawing::Point(205, 197);
         this->heightMapBrowseButton->Name = L"heightMapBrowseButton";
         this->heightMapBrowseButton->Size = System::Drawing::Size(75, 23);
         this->heightMapBrowseButton->TabIndex = 15;
         this->heightMapBrowseButton->Text = L"Browse...";
         this->heightMapBrowseButton->UseVisualStyleBackColor = true;
         // 
         // heightScaleUpDown
         // 
         this->heightScaleUpDown->Location = System::Drawing::Point(81, 232);
         this->heightScaleUpDown->Name = L"heightScaleUpDown";
         this->heightScaleUpDown->Size = System::Drawing::Size(120, 20);
         this->heightScaleUpDown->TabIndex = 18;
         // 
         // EditorMapSettings
         // 
         this->AcceptButton = this->okButton;
         this->CancelButton = this->cancelButton;
         this->ClientSize = System::Drawing::Size(292, 312);
         this->Controls->Add(this->heightScaleUpDown);
         this->Controls->Add(label6);
         this->Controls->Add(this->heightMapBrowseButton);
         this->Controls->Add(this->heightMapTextBox);
         this->Controls->Add(this->heightMapRadioButton);
         this->Controls->Add(this->genNoiseRadioButton);
         this->Controls->Add(this->uniformHeightRadioButton);
         this->Controls->Add(label5);
         this->Controls->Add(this->initialTileComboBox);
         this->Controls->Add(label4);
         this->Controls->Add(this->tileSetComboBox);
         this->Controls->Add(label3);
         this->Controls->Add(this->heightComboBox);
         this->Controls->Add(label2);
         this->Controls->Add(this->widthComboBox);
         this->Controls->Add(label1);
         this->Controls->Add(this->cancelButton);
         this->Controls->Add(this->okButton);
         this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
         this->MaximizeBox = false;
         this->MinimizeBox = false;
         this->Name = L"EditorMapSettings";
         this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
         this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
         this->Text = L"Map Settings";
         this->Load += gcnew System::EventHandler(this, &EditorMapSettingsDlg::OnLoad);
         (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->heightScaleUpDown))->EndInit();
         this->ResumeLayout(false);
         this->PerformLayout();

      }
#pragma endregion
private:
   System::Void OnValidated(System::Object ^ sender, System::EventArgs ^ e);
   System::Void OnLoad(System::Object ^ sender, System::EventArgs ^ e);
   EditorMapSettings ^ m_mapSettings;
};

   public ref class EditorMapSettings sealed
   {
   public:
      property int Width;
      property int Height;
      property String ^ TileSet;
      property int HeightScale;
   };

}
