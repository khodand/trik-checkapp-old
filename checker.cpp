#include "checker.h"

#include <QProcess>
#include <QDebug>
#include <QtConcurrent>
#include <QProgressDialog>
#include <QTime>

#include <optionsAliases.h>
#include <htmlTemplates.h>

const int BACKGROUND_TIMELIMIT = 20 * 1000;

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
	dialog.setCancelButtonText("Отмена");
	dialog.setWindowTitle("TRIK CheckApp");
	dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	dialog.setLabelText("Выполняется проверка...");


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
	if (dialog.wasCanceled()) {
		watcher.waitForFinished();
	}
	qDebug() << "watcher.isCanceled()" << watcher.isCanceled();

	for(auto &&t : tasksList) {
		delete t;
	}


}

QList<Checker::TaskReport> Checker::checkTask(const Checker::Task *t)
{
	QList<TaskReport> result;
	QElapsedTimer timer;
	for (auto &&f : t->fieldsInfos) {
		startProcess("patcher", QStringList(t->qrs.absoluteFilePath()) + t->patcherOptions + QStringList(f.absoluteFilePath()));

		TaskReport report;
		report.name = t->qrs.fileName();
		report.task = f.fileName();

		timer.restart();
		report.error = startProcess("2D-model", QStringList(t->qrs.absoluteFilePath()) + t->runnerOptions);
		qDebug() << report.name << report.task << report.error;
		report.time = QTime::fromMSecsSinceStartOfDay(timer.elapsed()).toString("mm:ss:zzz");
		if (!isErrorMessage(report.error)) {
			int start = report.error.indexOf("за") + 3;
			int end = report.error.indexOf("сек!") - 1;
			qDebug() << "REPORTER: " << start << end << report.error.mid(start, end - start);
			report.time += "/" + report.error.mid(start, end - start);
		}

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

	if (options.contains("-b")) {
		QTimer::singleShot(BACKGROUND_TIMELIMIT, &proccess, &QProcess::terminate);
	}

	auto p = program;
	proccess.start(p, options);
	if (!proccess.waitForStarted()) {
		qDebug() << "model" << "not started" << proccess.exitStatus();;
		return "Error: not started";
	}

	if (!proccess.waitForFinished()) {
		qDebug() << "model" << "not finished" << proccess.exitStatus();;
		return "Error: not finished";
	}

	QString error = proccess.readAllStandardError();

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
			numberOfCorrect[i] += isErrorMessage(r.error) ? 0 : 1;
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

		int counter = 0;
		QString name;
		for (auto &&r : studentResults) {
			name = "";
			if (counter == 0) {
				name = r.name;
			} else if (counter == 1) {
				color = "";
				name = QString("Итог %1 из %2").arg(numberOfCorrect[i]).arg(result[key].length());
			}
			qDebug() << r.name << r.task;
			qDebug() << r.error;
			QString status = isErrorMessage(r.error) ? "Ошибка" : "Выполнено";
			qDebug() << status;
			body += taskReport.arg(color).arg(name).arg(r.task).arg(status).arg(r.time);

			counter++;
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

	qDebug() << "REPORT CREATED";
	delete[] numberOfCorrect;
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

	result << "-m" << "script";
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

bool Checker::isErrorMessage(const QString &message)
{
	//return message.indexOf("Information") == -1;
	return message.indexOf("выполнено") == -1;
}
