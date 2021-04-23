#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>

#include "modelworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectTaskDirectory();

    void runCheck();

private:
    Ui::MainWindow *mUi;
    QDir mTasksDir;
    QDir mStudioDir;

    QList <QThread *> mWorkerThreads;
    QList <ModelWorker *> mModelWorkers;

    QAtomicInt mActiveModels {0};
};
#endif // MAINWINDOW_H
