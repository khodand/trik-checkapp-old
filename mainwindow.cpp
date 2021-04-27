#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtConcurrent/QtConcurrent>
#include <checker.h>
#include <QDebug>
#include <QFileDialog>
#include <QProgressDialog>
#include <QSettings>

#include <optionsAliases.h>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, mUi(new Ui::MainWindow)
	, mTasksDir(QDir::currentPath())
	, mLocalSettings(QDir::toNativeSeparators(mTasksDir.absolutePath() + "/taskCheck.ini"))
{
	mUi->setupUi(this);

	connect(mUi->backgroundOption, &QGroupBox::toggled, [this](bool state){
		mDirOptions[mTasksPath][backgroundOption] = !state;
		mUi->closeOnSuccessOption->setEnabled(state);
	});

	loadSettings();
}

MainWindow::~MainWindow()
{
	saveSettings();
	delete mUi;
}

void MainWindow::on_runCheckButton_clicked()
{
	auto qrsList = mTasksDir.entryInfoList({"*.qrs"}, QDir::Files);
	auto fields = mFieldsDir.entryInfoList({"*.xml"}, QDir::Files);

	Checker checker(mTasksPath);
	checker.revieweTasks(qrsList, fields, mDirOptions[mTasksPath]);
}

void MainWindow::on_chooseField_clicked()
{
	mFieldsDir = chooseDirectoryDialog();
	auto path = QDir::toNativeSeparators(mFieldsDir.absolutePath());
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

	auto xmlField = mDirOptions[mTasksPath][xmlFieldsDir].toString();
	mFieldsDir = xmlField.isEmpty() ? mTasksDir : QDir(xmlField);
	mDirOptions[mTasksPath][xmlFieldsDir] = xmlField.isEmpty() ? mTasksPath : xmlField;

	resetUiOptions(mDirOptions[mTasksPath]);
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

void MainWindow::on_closeOnSuccessOption_stateChanged(int state)
{
	mDirOptions[mTasksPath][closeSuccessOption] = state == Qt::CheckState::Checked;
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
