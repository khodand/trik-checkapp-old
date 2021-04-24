#include "modelworker.h"

#include <QProcess>
#include <QDebug>

ModelWorker::ModelWorker(const QString &fileName)
	: mName(fileName)
{
}

void ModelWorker::startExecution(const QString &qrsFile)
{
	QProcess patcher;
	patcher.start("patcher", {qrsFile, "--rrp"});
	if (!patcher.waitForStarted()){
		qDebug() << "patcher" << "not started";
		return;
	}

	if (!patcher.waitForFinished()) {
		qDebug() << "patcher" << "not finished";
		return;
	}

	auto result = QString(patcher.readAllStandardOutput());
	qDebug() << "patcher" << result;

	QProcess model;
	//model.start("2D-model", {qrsFile, "--close-on-succes"});
	model.start("cmd", {"/c", "ver"});
	if (!model.waitForStarted()) {
		qDebug() << "model" << "not started";
		return;
	}

	if (!model.waitForFinished()) {
		qDebug() << "model" << "not finished";
		return;
	}

	result = QString(model.readAll());
	qDebug() << result;

	//QProcess::startDetached("2D-model", {qrsFile, "--close-on-succes"});
	emit finished();
}
