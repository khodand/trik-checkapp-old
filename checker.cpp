#include "checker.h"

#include <QProcess>
#include <QDebug>
#include <QtConcurrent>
#include <QProgressDialog>

#include <optionsAliases.h>
#include <htmlTemplates.h>

Checker::Checker(const QString &tasksPath)
	: mTasksPath(tasksPath)
{
}

void Checker::revieweTasks(const QFileInfoList &qrsInfos, const QFileInfoList &fieldsInfos, const QHash<QString, QVariant> &options)
{
	auto patcherOptions = generatePathcerOptions(options);
	auto runnerOptions = generateRunnerOptions(options);

	QList<Task *> tasksList;
	for (auto &&qrs : qrsInfos) {
		tasksList += new Task({qrs, fieldsInfos, patcherOptions, runnerOptions});
	}

	QProgressDialog dialog;
	dialog.setLabelText(QString("%1 on %2 using %3")
						.arg(qrsInfos.length()).arg(fieldsInfos.length()).arg(QThread::idealThreadCount()));


	QFutureWatcher<QHash<QString, QList<TaskReport>>> watcher;
	connect(&dialog,  &QProgressDialog::canceled, &watcher
			, &QFutureWatcher<QHash<QString, QList<TaskReport>>>::cancel);
	connect(&watcher, &QFutureWatcher<QHash<QString, QList<TaskReport>>>::progressRangeChanged
			, &dialog, &QProgressDialog::setRange);
	connect(&watcher, &QFutureWatcher<QHash<QString, QList<TaskReport>>>::progressValueChanged
			, &dialog, &QProgressDialog::setValue);
	connect(&watcher, &QFutureWatcher<QHash<QString, QList<TaskReport>>>::finished,
			this, [this, &dialog, &watcher](){
		dialog.setLabelText("Создаю отчёт");
		if (!watcher.isCanceled()) {
			auto result = watcher.result();
			this->createHtmlReport(result);
		}
		dialog.reset();
	});

	auto futureTasks = QtConcurrent::mappedReduced(tasksList, checkTask, reduceFunction);
	watcher.setFuture(futureTasks);
	dialog.exec();
}

QList<Checker::TaskReport> Checker::checkTask(const Checker::Task *t)
{
	QList<TaskReport> result;
	for (auto &&f : t->fieldsInfos) {
		startProcess("patcher", QStringList(t->qrs.absoluteFilePath()) + t->patcherOptions + QStringList(f.absoluteFilePath()));

		TaskReport report;
		report.name = t->qrs.fileName();
		report.task = f.fileName();
		report.time = "0";
		report.error = startProcess("2D-model", QStringList(t->qrs.absoluteFilePath()) + t->runnerOptions);

		result.append(report);
	}

	return result;
}

void Checker::reduceFunction(QHash<QString, QList<TaskReport>> &result, const QList<TaskReport> &intermediate)
{
	for (auto i : intermediate) {
		result[i.name].append(i);
	}
}

QString Checker::startProcess(const QString &program, const QStringList &options)
{
	QProcess proccess;
	qDebug() << program << options << __PRETTY_FUNCTION__;
	proccess.start(program, options);
	if (!proccess.waitForStarted()) {
		qDebug() << "model" << "not started" << proccess.exitStatus();;
		return "Error: not started";
	}

	if (!proccess.waitForFinished()) {
		qDebug() << "model" << "not finished" << proccess.exitStatus();;
		return "Error: not finished";
	}

	auto error = proccess.readAllStandardError();
	qDebug() << proccess.readAll() << error << proccess.readAllStandardOutput() << proccess.exitStatus();
	return error;
}

void Checker::createHtmlReport(QHash<QString, QList<TaskReport>> &result)
{
	auto qrsNames = result.keys();
	auto numberOfCorrect = new int[qrsNames.length()] {0};
	std::sort(qrsNames.begin(), qrsNames.end());

	int i = 0;
	for (auto &&key : qrsNames) {
		std::sort(result[key].begin(), result[key].end(), compareReportsByTask);
		foreach (auto r, result[key]) {
			numberOfCorrect[i] += r.error.isEmpty() ? 1 : 0;
		}
		i++;
	}

	QFile reportFile(mTasksPath + QDir::separator() + "report.html");
	QFile htmlBegin(":/report_begin.html");
	QFile htmlEnd(":/report_end.html");

	QString body = reportHeader.arg(mTasksPath.section(QDir::separator(), -1)).arg(QDateTime::currentDateTime().toString("hh:mm dd.MM.yyyy"));

	i = 0;
	for (auto &&key : qrsNames) {
		auto studentResults = result[key];

		QString color = yellowCssClass;
		if (numberOfCorrect[i] == studentResults.length()) {
			color = greenCssClass;
		} else if (numberOfCorrect[i] == 0) {
			color = blackCssClass;
		}

		auto first = studentResults.first();
		body += taskReport.arg(color).arg(first.name).arg(first.task).arg(first.error.isEmpty() ? "Выполнено" : "Ошибка").arg("0");
		studentResults.removeFirst();

		if (!studentResults.isEmpty()) {
			first = studentResults.first();
			auto summary = QString("Итог %1 из %2").arg(numberOfCorrect[i]).arg(result[key].length());
			body += taskReport.arg("").arg(summary).arg(first.task).arg(first.error.isEmpty() ? "Выполнено" : "Ошибка").arg("0");
			studentResults.removeFirst();

			for (auto &&r : studentResults) {
				body += taskReport.arg("").arg("").arg(r.task).arg(r.error.isEmpty() ? "Выполнено" : "Ошибка").arg("0");
			}
		}

		i++;
	}

	htmlBegin.open(QFile::ReadOnly);
	QString report = htmlBegin.readAll();
	htmlBegin.close();

	report += body;

	htmlEnd.open(QFile::ReadOnly);
	report += htmlEnd.readAll();
	htmlEnd.close();

	std::string html = report.toStdString();
	const auto raw = html.c_str();
	reportFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
	reportFile.write(raw);
	reportFile.close();
}

const QStringList Checker::generateRunnerOptions(const QHash<QString, QVariant> &options)
{
	QStringList result;

	qDebug() << options;
	if (options[closeSuccessOption].toBool())
		result << "--close-on-succes";

	if (options[backgroundOption].toBool())
		result << "-b";

	if (options[consoleOption].toBool())
		result << "-c";

	return result;
}

const QStringList Checker::generatePathcerOptions(const QHash<QString, QVariant> &options)
{
	QStringList result;

	if (options[patchField].toBool()) {
		result << "-f";
	}
	else {
		if (options[patchWP].toBool()) {
				result << "--wp";
		}
		else {
			if (options[resetRP].toBool()) {
				result << "--rrp";
			}
			if (options[patchWorld].toBool()) {
				result << "-w";
			}
		}
	}

	return result;
}
