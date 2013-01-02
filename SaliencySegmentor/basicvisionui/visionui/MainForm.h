#pragma once


#pragma managed(push, off)
#include <opencv2/opencv.hpp>
#pragma managed(pop)

namespace visionui {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class SearchForm : public System::Windows::Forms::Form
	{
	public:
		SearchForm(void)
		{
			InitializeComponent();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SearchForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::PictureBox^  queryImgBox;
	private: System::Windows::Forms::TextBox^  queryImgPathTextBox;
	private: System::Windows::Forms::Button^  loadImgBtn;
	private: System::Windows::Forms::Button^  runBtn;
	

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
			this->queryImgBox = (gcnew System::Windows::Forms::PictureBox());
			this->queryImgPathTextBox = (gcnew System::Windows::Forms::TextBox());
			this->loadImgBtn = (gcnew System::Windows::Forms::Button());
			this->runBtn = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->queryImgBox))->BeginInit();
			this->SuspendLayout();
			// 
			// queryImgBox
			// 
			this->queryImgBox->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->queryImgBox->Location = System::Drawing::Point(12, 12);
			this->queryImgBox->Name = L"queryImgBox";
			this->queryImgBox->Size = System::Drawing::Size(330, 293);
			this->queryImgBox->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->queryImgBox->TabIndex = 0;
			this->queryImgBox->TabStop = false;
			// 
			// queryImgPathTextBox
			// 
			this->queryImgPathTextBox->Location = System::Drawing::Point(12, 326);
			this->queryImgPathTextBox->Name = L"queryImgPathTextBox";
			this->queryImgPathTextBox->Size = System::Drawing::Size(330, 20);
			this->queryImgPathTextBox->TabIndex = 4;
			// 
			// loadImgBtn
			// 
			this->loadImgBtn->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->loadImgBtn->Location = System::Drawing::Point(55, 362);
			this->loadImgBtn->Name = L"loadImgBtn";
			this->loadImgBtn->Size = System::Drawing::Size(103, 29);
			this->loadImgBtn->TabIndex = 3;
			this->loadImgBtn->Text = L"Select Image";
			this->loadImgBtn->UseVisualStyleBackColor = true;
			this->loadImgBtn->Click += gcnew System::EventHandler(this, &SearchForm::loadImgBtn_Click);
			// 
			// runBtn
			// 
			this->runBtn->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->runBtn->Location = System::Drawing::Point(183, 362);
			this->runBtn->Name = L"runBtn";
			this->runBtn->Size = System::Drawing::Size(103, 29);
			this->runBtn->TabIndex = 3;
			this->runBtn->Text = L"Run";
			this->runBtn->UseVisualStyleBackColor = true;
			this->runBtn->Click += gcnew System::EventHandler(this, &SearchForm::runBtn_Click);
			// 
			// SearchForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(367, 446);
			this->Controls->Add(this->queryImgPathTextBox);
			this->Controls->Add(this->runBtn);
			this->Controls->Add(this->loadImgBtn);
			this->Controls->Add(this->queryImgBox);
			this->Name = L"SearchForm";
			this->Text = L"ImagePicker";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->queryImgBox))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

	private: System::Void loadImgBtn_Click(System::Object^  sender, System::EventArgs^  e) {

				 OpenFileDialog^ openImgDialog = gcnew OpenFileDialog();
				 openImgDialog->InitialDirectory = "D:\\";
				 openImgDialog->Filter = "Image Files(*.BMP;*.JPG;*.PNG;*.GIF)|*.BMP;*.JPG;*.GIF;*.PNG";
				 openImgDialog->FilterIndex = 1;
				 openImgDialog->RestoreDirectory = true;

				 if( openImgDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK )
				 {
					 // image path
					 System::String^ imgpath = openImgDialog->FileName;
					 queryImgPathTextBox->Text = openImgDialog->FileName;
					 // load in box
					 queryImgBox->Load(imgpath);
				 }

			 }

private: System::Void runBtn_Click(System::Object^  sender, System::EventArgs^  e) {



		 }
};
}

