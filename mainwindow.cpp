#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <QSettings>
#include <QTextBrowser>
#include <QThread>
#include <QtCore/QEventLoop>

const QString closeSuccessOption = "closeOnSuccess";
const QString backgroundOption = "background";
const QString speedOption = "speed";
const QString consoleOption = "showConsole";

const QString xmlFieldPath = "xmlPath";
const QString patchField = "patchField";
const QString patchWorld = "patchWorld";
const QString patchWP = "patchWroldAndPosition";
const QString resetRP = "resetRobotPosition";

const QHash <QString, QVariant> defaultOptions {{closeSuccessOption, true}
                                               ,{backgroundOption, false}
                                               ,{speedOption, 5}
                                               ,{consoleOption, false}
                                               ,{xmlFieldPath, ""}
                                               ,{patchField, true}
                                               ,{patchWorld, false}
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
    connect(mUi->backgroundOption, &QCheckBox::stateChanged, [this](int state){
        mDirOptions[mTasksPath][backgroundOption] = state == Qt::CheckState::Checked;
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

QString MainWindow::chooseFile()
{
    QFileDialog dialog;
    dialog.setNameFilter("*.xml");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.exec();
    return dialog.directory().absoluteFilePath(dialog.selectedFiles().first());
}

void MainWindow::resetUiOptions(const QHash<QString, QVariant> &options)
{
    options[closeSuccessOption].toBool() ? mUi->closeOnSuccessOption->setCheckState(Qt::CheckState::Checked)
                                         : mUi->closeOnSuccessOption->setCheckState(Qt::CheckState::Unchecked);
    options[backgroundOption].toBool() ? mUi->backgroundOption->setCheckState(Qt::CheckState::Checked)
                                         : mUi->backgroundOption->setCheckState(Qt::CheckState::Unchecked);
    options[patchField].toBool() ? mUi->wPPCheckBox->setCheckState(Qt::CheckState::Checked)
                                         : mUi->wPPCheckBox->setCheckState(Qt::CheckState::Unchecked);
    options[patchWorld].toBool() ? mUi->worldCheckBox->setCheckState(Qt::CheckState::Checked)
                                         : mUi->worldCheckBox->setCheckState(Qt::CheckState::Unchecked);
    options[patchWP].toBool() ? mUi->wPcheckBox->setCheckState(Qt::CheckState::Checked)
                                         : mUi->wPcheckBox->setCheckState(Qt::CheckState::Unchecked);
    options[resetRP].toBool() ? mUi->resetPCheckBox->setCheckState(Qt::CheckState::Checked)
                                         : mUi->resetPCheckBox->setCheckState(Qt::CheckState::Unchecked);
    options[consoleOption].toBool() ? mUi->resetPCheckBox->setCheckState(Qt::CheckState::Checked)
                                         : mUi->resetPCheckBox->setCheckState(Qt::CheckState::Unchecked);
    mUi->xmlFieldPath->setText(options[xmlFieldPath].toString());

	//mUi->speedLineEdit->setText(options[speedOption].toString());
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
        qDebug() << "beginGroup" << dir;
        settings.beginGroup(dir);
		for (auto &&option: mDirOptions[dir].keys()) {
            qDebug() << option << mDirOptions[dir][option];
            settings.setValue(option, mDirOptions[dir][option]);
        }
        settings.endGroup();
    }
}

const QStringList MainWindow::generateRunnerOptions(const QString &file)
{
    QStringList result {file};

    qDebug() << mDirOptions[mTasksPath];
    if (mDirOptions[mTasksPath][closeSuccessOption].toBool())
        result << "--close-on-succes";

    if (mDirOptions[mTasksPath][backgroundOption].toBool())
        result << "-b";

    if (mDirOptions[mTasksPath][consoleOption].toBool())
        result << "-c";

    result << "-s" << mDirOptions[mTasksPath][speedOption].toString();

    result.append("-m");
    result.append("script");
    qDebug() << result;
    return result;
}

const QStringList MainWindow::generatePathcerOptions(const QString &file)
{
    QStringList result {file};
    auto xml = mDirOptions[mTasksPath][xmlFieldPath].toString();

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
    connect(&proccess, &QProcess::readyReadStandardError
            , this, [&proccess](){qDebug() << proccess.readAllStandardError();});
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
    auto path = chooseFile();
    mUi->xmlFieldPath->setText(QDir::toNativeSeparators(path));
    qDebug() << QDir::toNativeSeparators(path);
    mDirOptions[mTasksPath][xmlFieldPath] = path;
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
    auto tasks = mTasksDir.entryList({"*.qrs"}, QDir::Files);
    qDebug() << tasks;

	for (auto &&t : tasks) {
        /*mActiveModels++;
        auto workerThread = new QThread();
        mWorkerThreads.append(workerThread);

        auto worker = new ModelWorker(t);
        mModelWorkers.append(worker);

        worker->moveToThread(workerThread);
        connect(worker, &ModelWorker::finished, this, [this](){mActiveModels--;});
        connect(workerThread, &QThread::started, worker, [=](){worker->startExecution(mTasksDir.filePath(t));});
        workerThread->start();*/

        mTasksStatus.append({t, executeProcess("2D-model", generateRunnerOptions(mTasksDir.filePath(t)))});
    }
    createReport();
}

void MainWindow::on_patcherButton_clicked()
{
    auto toPatch = mTasksDir.entryList({"*.qrs"}, QDir::Files);
	for (auto &&qrs : toPatch) {
        executeProcess("patcher", generatePathcerOptions(mTasksDir.filePath(qrs)));
    }
}

void MainWindow::on_wPcheckBox_stateChanged(int state)
{
    mDirOptions[mTasksPath][patchWP] = state == Qt::CheckState::Checked;
}

void MainWindow::on_worldCheckBox_stateChanged(int state)
{
    mDirOptions[mTasksPath][patchWorld] = state == Qt::CheckState::Checked;
}

void MainWindow::on_wPPCheckBox_stateChanged(int state)
{
    mDirOptions[mTasksPath][patchField] = state == Qt::CheckState::Checked;
}

void MainWindow::on_resetPCheckBox_stateChanged(int state)
{
    mDirOptions[mTasksPath][resetRP] = state == Qt::CheckState::Checked;
}

void MainWindow::on_speedLineEdit_textChanged(const QString &speed)
{
    bool ok;
    int speedValue = speed.toInt(&ok);
    if (ok) {
        mDirOptions[mTasksPath][speedOption] = speedValue;
    }
}

void MainWindow::on_showConsolecheckBox_stateChanged(int state)
{
     mDirOptions[mTasksPath][consoleOption] = state == Qt::CheckState::Checked;
}
