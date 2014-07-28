#ifndef SMARTWINDOWUI_H
#define SMARTWINDOWUI_H

#include <QtWidgets/QMainWindow>
#include "ui_smartwindowui.h"

class SmartWindowUI : public QMainWindow
{
	Q_OBJECT

public:
	SmartWindowUI(QWidget *parent = 0);

private:
	Ui::SmartWindowUIClass ui;

private slots:
	void on_openImgBtn_clicked();

};

#endif // SMARTWINDOWUI_H
