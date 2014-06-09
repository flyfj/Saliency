#include "smartwindowui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SmartWindowUI w;
	w.show();
	return a.exec();
}
