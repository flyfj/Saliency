#include <QtWidgets>
#include "smartwindowui.h"

SmartWindowUI::SmartWindowUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

//////////////////////////////////////////////////////////////////////////

void SmartWindowUI::on_openImgBtn_clicked()
{
	QString imgfile = QFileDialog::getOpenFileName(this,
		tr("Open Image Files"), "E:\\datasets\\features\\", tr("Image Files (*.jpg *.png)"));

	if(imgfile.length() == 0)
		return;

	ui.in_imgpath_textedit->setText(imgfile);

	// set up image
	QImage img(imgfile);
	ui.in_img_label->setPixmap(QPixmap::fromImage(img));
}
