#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);
	qDebug() << QStyleFactory::keys();
	a.setStyle(QStyleFactory::create("Fusion"));

	MainWindow w;
	w.show();
	return a.exec();
}
