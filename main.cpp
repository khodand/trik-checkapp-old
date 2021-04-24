#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	qDebug() << QStyleFactory::keys();
	a.setStyle(QStyleFactory::create("Fusion"));

	MainWindow w;
	w.show();
	return a.exec();
}
