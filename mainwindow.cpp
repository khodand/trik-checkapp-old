#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <QThread>
#include <QtCore/QEventLoop>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mUi(new Ui::MainWindow)
{
    mUi->setupUi(this);

    mTasksDir = QDir::currentPath();
    mStudioDir = QDir::currentPath();

    connect(mUi->openTasks, &QPushButton::clicked, this, &MainWindow::selectTaskDirectory);
    connect(mUi->runCheckButton, &QPushButton::clicked, this, &MainWindow::runCheck);
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::selectTaskDirectory()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setOption(QFileDialog::ShowDirsOnly, false);
    dialog.exec();
    mTasksDir = dialog.directory();
    mUi->currentTasksDir->setText(mTasksDir.absolutePath());
    qDebug() << mTasksDir;
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

