#ifndef SMARTWINDOWUI_H
#define SMARTWINDOWUI_H

#include <QtWidgets/QMainWindow>
#include "ui_smartwindowui.h"

class SmartWindowUI : public QMainWindow
{
	Q_OBJECT

public:
	SmartWindowUI(QWidget *parent = 0);
	~SmartWindowUI();

private:
	Ui::SmartWindowUIClass ui;
};

#endif // SMARTWINDOWUI_H
