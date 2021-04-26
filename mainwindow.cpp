#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include <QFileDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QProcess>
#include <QProgressDialog>
#include <QSettings>
#include <QTextBrowser>
#include <QThread>
#include <QtCore/QEventLoop>

const QString closeSuccessOption = "closeOnSuccess";
const QString backgroundOption = "background";
const QString consoleOption = "showConsole";

const QString xmlFieldsDir = "fieldsDir";
const QString patchField = "patchField";
const QString patchWorld = "patchWorld";
const QString patchWP = "patchWroldAndPosition";
const QString resetRP = "resetRobotPosition";

const QHash <QString, QVariant> defaultOptions {{closeSuccessOption, true}
											   ,{backgroundOption, false}
											   ,{consoleOption, false}
											   ,{xmlFieldsDir, ""}
											   ,{patchField, true}
											   ,{patchWorld, true}
											   ,{patchWP, false}};

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, mUi(new Ui::MainWindow)
	, mTasksDir(QDir::currentPath())
	, mLocalSettings(QDir::toNativeSeparators(mTasksDir.absolutePath() + "/taskCheck.ini"))
{
	mUi->setupUi(this);

	connect(mUi->closeOnSuccessOption, &QCheckBox::stateChanged, [this](int state){
		mDirOptions[mTasksPath][closeSuccessOption] = state == Qt::CheckState::Checked;
	});
	connect(mUi->backgroundOption, &QGroupBox::toggled, [this](bool state){
		mDirOptions[mTasksPath][backgroundOption] = !state;
	});

	loadSettings();
}

MainWindow::~MainWindow()
{
	saveSettings();
	for (auto &&i : mModelWorkers){
		i->deleteLater();
	}
	for (auto &&i : mWorkerThreads){
		i->deleteLater();
	}
	delete mUi;
}

QDir MainWindow::chooseDirectoryDialog()
{
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::Directory);
	dialog.exec();
	return dialog.directory();
}

void MainWindow::resetUiOptions(const QHash<QString, QVariant> &options)
{
	options[closeSuccessOption].toBool() ? mUi->closeOnSuccessOption->setCheckState(Qt::CheckState::Checked)
										 : mUi->closeOnSuccessOption->setCheckState(Qt::CheckState::Unchecked);
	mUi->backgroundOption->setChecked(!options[backgroundOption].toBool());
	options[patchField].toBool() ? mUi->wPPCheckBox->setCheckState(Qt::CheckState::Checked)
										 : mUi->wPPCheckBox->setCheckState(Qt::CheckState::Unchecked);
	options[patchWP].toBool() ? mUi->wPcheckBox->setCheckState(Qt::CheckState::Checked)
										 : mUi->wPcheckBox->setCheckState(Qt::CheckState::Unchecked);
	options[resetRP].toBool() ? mUi->resetPCheckBox->setCheckState(Qt::CheckState::Checked)
										 : mUi->resetPCheckBox->setCheckState(Qt::CheckState::Unchecked);
	options[consoleOption].toBool() ? mUi->resetPCheckBox->setCheckState(Qt::CheckState::Checked)
										 : mUi->resetPCheckBox->setCheckState(Qt::CheckState::Unchecked);
	mUi->xmlFieldsDir->setText(options[xmlFieldsDir].toString());
}

void MainWindow::loadSettings()
{
	qDebug() << mLocalSettings;
	QSettings settings(mLocalSettings, QSettings::IniFormat);
	auto groups = settings.childGroups();
	qDebug() << groups;
	for (auto &&g : groups) {
		QHash <QString, QVariant> options;

		settings.beginGroup(g);
		for (auto &&key : defaultOptions.keys()) {
			options[key] = settings.value(key, defaultOptions[key]);
		}
		settings.endGroup();

		mDirOptions[g] = options;
	}
}

void MainWindow::saveSettings()
{
	QSettings settings(mLocalSettings, QSettings::IniFormat);
	for (auto &&dir: mDirOptions.keys()) {
		settings.beginGroup(dir);
		for (auto &&option: mDirOptions[dir].keys()) {
			qDebug() << option << mDirOptions[dir][option];
			settings.setValue(option, mDirOptions[dir][option]);
		}
		settings.endGroup();
	}
}


// For now we cant run tasks with javascript or python. Only scheme.
const QStringList MainWindow::generateRunnerOptions(const QString &file)
{
	QStringList result;// {file};

	qDebug() << mDirOptions[mTasksPath];
	if (mDirOptions[mTasksPath][closeSuccessOption].toBool())
		result << "--close-on-succes";

	if (mDirOptions[mTasksPath][backgroundOption].toBool())
		result << "-b";

	if (mDirOptions[mTasksPath][consoleOption].toBool())
		result << "-c";

//	result.append("-m");
//	result.append("script");
	qDebug() << result;
	return result;
}

const QStringList MainWindow::generatePathcerOptions(const QString &file)
{
	QStringList result;// {file};
	auto xml = mDirOptions[mTasksPath][xmlFieldsDir].toString() + "\\tast_4.xml";

	if (mDirOptions[mTasksPath][patchField].toBool()) {
		result << "-f" << xml;
	}
	else {
		if (mDirOptions[mTasksPath][patchWP].toBool()) {
				result << "--wp" << xml;
		}
		else {
			if (mDirOptions[mTasksPath][patchWorld].toBool()) {
				result << "-w" << xml;
			}
			if (mDirOptions[mTasksPath][resetRP].toBool()) {
				result << "--rrp";
			}
		}
	}

	qDebug() << result;
	return result;
}

QString MainWindow::executeProcess(const QString &program, const QStringList &options)
{
	QProcess proccess;
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


void MainWindow::on_chooseField_clicked()
{
	auto path = QDir::toNativeSeparators(chooseDirectoryDialog().absolutePath());
	path = QDir::toNativeSeparators(path);
	mUi->xmlFieldsDir->setText(path);
	qDebug() << path;
	mDirOptions[mTasksPath][xmlFieldsDir] = path;
}

void MainWindow::on_openTasks_clicked()
{
	mTasksDir = chooseDirectoryDialog();
	mTasksPath = QDir::toNativeSeparators(mTasksDir.absolutePath());
	mUi->currentTasksDir->setText(mTasksPath);

	if (!mDirOptions.contains(mTasksPath)) {
		mDirOptions[mTasksPath] = defaultOptions;
	}
	resetUiOptions(mDirOptions[mTasksPath]);
}

void MainWindow::createReport()
{
	QFile report(mTasksPath + QDir::separator() + "report.html");
	QString head = "<!DOCTYPE html><html> <head> <meta charset=\"utf-8\" /> <title>Report</title> <style> "
						 "article, aside, details, figcaption, figure, footer,header, hgroup, menu, nav, section "
						 "{ display: block; } </style> <img src=\"trik_logo.png\" width=\"50%\" style=\"display: "
						 "block; margin-left: auto; margin-right: auto;\"/> <link rel=\"preconnect\" href=\""
						 "https://fonts.gstatic.com\"> <link href=\"https://fonts.googleapis.com/css2?family="
						 "Montserrat:wght@600&display=swap\" rel=\"stylesheet\"> </head> <body style=\"font-size:"
						 " larger; font-family: 'Montserrat', sans-serif;\">";
	QString end = "</body></html>";
	QString body = "";

	for (auto &&s : mTasksStatus) {
		if (s.second.isEmpty()) {
			body += QString("<p style=\"color: #009933;\">%1 Successfull!</p>").arg(s.first);
		}
		else {
			body += QString("<p style=\"color: #001a42;\">%1 Error: %2</p>").arg(s.first, s.second);
		}
	}
	head += body + end;
	std::string html = head.toStdString();
	const char* raw = html.c_str();
	report.open(QIODevice::WriteOnly | QIODevice::Truncate);
	report.write(raw);
	report.close();
}


void MainWindow::on_runCheckButton_clicked()
{
	/*QList<QFuture<MainWindow::TaskReport>> futureTasks;
	auto tasks = mTasksDir.entryList({"*.qrs"}, QDir::Files);
	for (auto &&qrs : tasks) {

		auto patcherOptions = generatePathcerOptions(mTasksDir.filePath(qrs));
		auto runnerOptions = generateRunnerOptions(mTasksDir.filePath(qrs));
		futureTasks.append(QtConcurrent::run(runCheck, patcherOptions, runnerOptions));
	}

	for (auto &&f : futureTasks) {
		f.waitForFinished();
	}*/
	auto tasks = mTasksDir.entryList({"*.qrs"}, QDir::Files);

//	QProgressDialog dialog;
//	dialog.setLabelText(QString("Выполняем проверку %1 задач").arg(tasks.length()));

//	QFutureWatcher<QList<MainWindow::TaskReport>> watcher;
//	connect(&watcher, &QFutureWatcher<QList<MainWindow::TaskReport>>::finished, &dialog, &QProgressDialog::reset);
//	connect(&dialog,  &QProgressDialog::canceled, &watcher, &QFutureWatcher<QList<MainWindow::TaskReport>>::cancel);
//	connect(&watcher, &QFutureWatcher<QList<MainWindow::TaskReport>>::progressRangeChanged
//			, &dialog, &QProgressDialog::setRange);
//	connect(&watcher, &QFutureWatcher<QList<MainWindow::TaskReport>>::progressValueChanged
//			, &dialog, &QProgressDialog::setValue);

	auto patcherOptions = generatePathcerOptions("");
	auto runnerOptions = generateRunnerOptions("");
//	QFuture<QList<MainWindow::TaskReport>> allTasks = QtConcurrent::run(runAllChecks, tasks, patcherOptions, runnerOptions);
////	watcher.setFuture(allTasks);

////	dialog.exec();
////	watcher.waitForFinished();
//	allTasks.waitForFinished();
//	qDebug() << "end";

	QList<QFuture<MainWindow::TaskReport>> futureTasks;
	auto p = patcherOptions;
	for (auto &&qrs : tasks) {

		auto patcherOptions = generatePathcerOptions(mTasksDir.filePath(qrs));
		auto runnerOptions = generateRunnerOptions(mTasksDir.filePath(qrs));
		futureTasks.append(QtConcurrent::run(runCheck, qrs, patcherOptions, runnerOptions));
	}
//	for (const auto &qrs : tasks) {
//		qDebug() << "patcher" << patcherOptions;
//		qDebug() << "runner" << runnerOptions;
//		futureTasks.append(QtConcurrent::run(runCheck, qrs, patcherOptions, runnerOptions));
//	}

	QList<MainWindow::TaskReport> result;
	for (auto &&f : futureTasks) {
		f.waitForFinished();
		result.append(f.result());
	}

	createReport();
}

QList<MainWindow::TaskReport> MainWindow::runAllChecks(const QStringList &tasks, QStringList patcherOptions
													   , QStringList runnerOptions)
{
	QList<QFuture<MainWindow::TaskReport>> futureTasks;
	auto p = patcherOptions;
	for (auto &&qrs : tasks) {
		patcherOptions.prepend(qrs);
		runnerOptions.prepend(qrs);

		qDebug() << "patcher" << patcherOptions;
		qDebug() << "runner" << runnerOptions;
		futureTasks.append(QtConcurrent::run(runCheck, qrs, patcherOptions, runnerOptions));
		patcherOptions.removeFirst();
		runnerOptions.removeFirst();
	}

//	futureTasks.first().waitForFinished();
//	auto a = futureTasks.at(1);
//	a.waitForFinished();
	futureTasks.back().waitForFinished();

	QList<MainWindow::TaskReport> result;
	for (auto &&f : futureTasks) {
		f.waitForFinished();
		qDebug() << f.result().error;
	}

	return result;
}

MainWindow::TaskReport MainWindow::runCheck(const QString &name, const QStringList &patcherOptions
											, const QStringList &modelOptions)
{
	auto p = patcherOptions;
	p.prepend(name);
	qDebug() << "p" << p;
	executeProcess("patcher", p);

	TaskReport report;
	report.name = name;
	report.time = "0";
	auto m = modelOptions;
	m.prepend(name);
	qDebug() << "m" << m;
	report.error = executeProcess("2D-model", m);

	return report;
}

void MainWindow::on_wPcheckBox_stateChanged(int state)
{
	mDirOptions[mTasksPath][patchWP] = state == Qt::CheckState::Checked;
}

void MainWindow::on_wPPCheckBox_stateChanged(int state)
{
	mDirOptions[mTasksPath][patchField] = state == Qt::CheckState::Checked;
}

void MainWindow::on_resetPCheckBox_stateChanged(int state)
{
	mDirOptions[mTasksPath][resetRP] = state == Qt::CheckState::Checked;
}

void MainWindow::on_showConsoleCheckBox_stateChanged(int state)
{
	mDirOptions[mTasksPath][consoleOption] = state == Qt::CheckState::Checked;
}
