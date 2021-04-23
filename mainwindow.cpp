#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <QSettings>
#include <QThread>
#include <QtCore/QEventLoop>

const QString closeSuccessOption = "closeOnSuccess";
const QHash <QString, bool> defaultOptions {{closeSuccessOption, true}};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mUi(new Ui::MainWindow)
    , mTasksDir(QDir::currentPath())
    , mLocalSettings(QDir::toNativeSeparators(mTasksDir.absolutePath() + "/taskCheck.ini"))
{
    mUi->setupUi(this);

    mStudioDir = QDir::currentPath();

    connect(mUi->openTasks, &QPushButton::clicked, this, &MainWindow::selectTaskDirectory);
    connect(mUi->runCheckButton, &QPushButton::clicked, this, &MainWindow::runCheck);

    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
    for (auto i : mModelWorkers){
        i->deleteLater();
    }
    for (auto i : mWorkerThreads){
        i->deleteLater();
    }
    delete mUi;
}

void MainWindow::selectTaskDirectory()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setOption(QFileDialog::ShowDirsOnly, false);
    dialog.exec();
    mTasksDir = dialog.directory();
    auto path = QDir::toNativeSeparators(mTasksDir.absolutePath());
    mUi->currentTasksDir->setText(path);

    if (!mDirOptions.contains(path)) {
        mDirOptions[path] = defaultOptions;
    }
    resetUiOptions(mDirOptions[path]);
}

void MainWindow::runCheck()
{
    auto tasks = mTasksDir.entryList({"*.qrs"}, QDir::Files);
    qDebug() << tasks;

    for (auto t : tasks) {
        mActiveModels++;
        auto workerThread = new QThread();
        mWorkerThreads.append(workerThread);

        auto worker = new ModelWorker(t);
        mModelWorkers.append(worker);

        worker->moveToThread(workerThread);
        connect(worker, &ModelWorker::finished, this, [this](){mActiveModels--;});
        connect(workerThread, &QThread::started, worker, [=](){worker->startExecution(mTasksDir.filePath(t));});
        workerThread->start();
    }

}

void MainWindow::resetUiOptions(const QHash<QString, bool> &options)
{
    options[closeSuccessOption] ? mUi->closeOnSuccessOption->setCheckState(Qt::CheckState::Checked)
                                : mUi->closeOnSuccessOption->setCheckState(Qt::CheckState::Unchecked);
}

void MainWindow::loadSettings()
{
    qDebug() << mLocalSettings;
    QSettings settings(mLocalSettings, QSettings::IniFormat);
    auto groups = settings.childGroups();
    qDebug() << groups;
    for (auto g : groups) {
        QHash <QString, bool> options;

        settings.beginGroup(g);
        options[closeSuccessOption] = settings.value(closeSuccessOption, true).toBool();
        settings.endGroup();

        mDirOptions[g] = options;
    }
}

void MainWindow::saveSettings()
{
    QSettings settings(mLocalSettings, QSettings::IniFormat);
    for (auto &dir: mDirOptions.keys()) {
        qDebug() << "beginGroup" << dir;
        settings.beginGroup(dir);
        for (auto &option: mDirOptions[dir].keys()) {
            qDebug() << option << mDirOptions[dir][option];
            settings.setValue(option, mDirOptions[dir][option]);
        }
        settings.endGroup();
    }
}

