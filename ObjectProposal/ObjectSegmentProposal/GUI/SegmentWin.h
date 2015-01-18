#pragma once

#include "../stdafx.h"
#include <vcclr.h>
#include <msclr/marshal_cppstd.h>

#include "../ObjectProposalTester.h"

namespace ObjectSegmentProposal {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SegmentWin
	/// </summary>
	public ref class SegmentWin : public System::Windows::Forms::Form
	{
	public:
		SegmentWin(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			img_loaded = FALSE;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SegmentWin()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::PictureBox^  sel_img_box;
	private: System::Windows::Forms::Button^  openBtn;
	private: System::Windows::Forms::Button^  SegmentBtn;
	private: System::Boolean img_loaded;
	private: System::String^ img_path;


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
			this->sel_img_box = (gcnew System::Windows::Forms::PictureBox());
			this->openBtn = (gcnew System::Windows::Forms::Button());
			this->SegmentBtn = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->sel_img_box))->BeginInit();
			this->SuspendLayout();
			// 
			// sel_img_box
			// 
			this->sel_img_box->Location = System::Drawing::Point(92, 24);
			this->sel_img_box->Name = L"sel_img_box";
			this->sel_img_box->Size = System::Drawing::Size(337, 523);
			this->sel_img_box->SizeMode = System::Windows::Forms::PictureBoxSizeMode::AutoSize;
			this->sel_img_box->TabIndex = 0;
			this->sel_img_box->TabStop = false;
			this->sel_img_box->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &SegmentWin::sel_img_box_MouseDown);
			this->sel_img_box->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &SegmentWin::sel_img_box_MouseMove);
			this->sel_img_box->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &SegmentWin::sel_img_box_MouseUp);
			// 
			// openBtn
			// 
			this->openBtn->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->openBtn->Location = System::Drawing::Point(131, 587);
			this->openBtn->Name = L"openBtn";
			this->openBtn->Size = System::Drawing::Size(93, 41);
			this->openBtn->TabIndex = 1;
			this->openBtn->Text = L"Open";
			this->openBtn->UseVisualStyleBackColor = true;
			this->openBtn->Click += gcnew System::EventHandler(this, &SegmentWin::openBtn_Click);
			// 
			// SegmentBtn
			// 
			this->SegmentBtn->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->SegmentBtn->Location = System::Drawing::Point(289, 587);
			this->SegmentBtn->Name = L"SegmentBtn";
			this->SegmentBtn->Size = System::Drawing::Size(108, 41);
			this->SegmentBtn->TabIndex = 2;
			this->SegmentBtn->Text = L"Segment";
			this->SegmentBtn->UseVisualStyleBackColor = true;
			this->SegmentBtn->Click += gcnew System::EventHandler(this, &SegmentWin::SegmentBtn_Click);
			// 
			// SegmentWin
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(517, 684);
			this->Controls->Add(this->SegmentBtn);
			this->Controls->Add(this->openBtn);
			this->Controls->Add(this->sel_img_box);
			this->Name = L"SegmentWin";
			this->Text = L"SegmentWin";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->sel_img_box))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

		Bitmap^ ConvertMat2Image(cv::Mat& mat) {
			Bitmap^ img = gcnew Bitmap(mat.cols, mat.rows);
			for (int r = 0; r < mat.rows; r++) {
				for (int c = 0; c < mat.cols; c++) {
					Vec3b val = mat.at<Vec3b>(r, c);
					img->SetPixel(c, r, Color::FromArgb(val.val[2], val.val[1], val.val[0]));
				}
			}
			return img;
		}

	private: System::Void openBtn_Click(System::Object^  sender, System::EventArgs^  e) {
		OpenFileDialog^ file_dialog = gcnew OpenFileDialog;
		file_dialog->Filter = "Image Files(*.jpg)|*.jpg;*.jpeg";
		if (file_dialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
			img_path = gcnew System::String(file_dialog->FileName);
			cv::Mat img = cv::imread(msclr::interop::marshal_as<std::string>(file_dialog->FileName));
			cv::Size newsz;
			ToolFactory::compute_downsample_ratio(cv::Size(img.cols, img.rows), 400, newsz);
			cv::resize(img, img, newsz);
			Bitmap^ load_img = ConvertMat2Image(img);
			sel_img_box->Image = load_img;
			img_loaded = true;
		}
	}
	private: System::Void SegmentBtn_Click(System::Object^  sender, System::EventArgs^  e) {
		Bitmap^ img = nullptr;
	}
	private: System::Void sel_img_box_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
		if (!img_loaded) return;
		// get click location
		//MessageBox::Show(e->Location.ToString());
		System::String^ newstr = gcnew System::String(img_path);
		cv::Mat img = cv::imread(msclr::interop::marshal_as<std::string>(newstr));
		cv::Size newsz;
		ToolFactory::compute_downsample_ratio(cv::Size(img.cols, img.rows), 400, newsz);
		cv::resize(img, img, newsz);
		ObjectProposalTester tester;
		tester.TestPointSegment(img, cv::Point(e->Location.X, e->Location.Y));
	}
	private: System::Void sel_img_box_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

	}
	private: System::Void sel_img_box_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

	}
};
}
