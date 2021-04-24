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
    void on_chooseField_clicked();

    void on_openTasks_clicked();

    void on_runCheckButton_clicked();

    void on_patcherButton_clicked();

    void on_worldCheckBox_stateChanged(int state);

    void on_wPcheckBox_stateChanged(int state);

    void on_wPPCheckBox_stateChanged(int state);

    void on_resetPCheckBox_stateChanged(int state);

    void on_speedLineEdit_textChanged(const QString &speed);

    void on_showConsolecheckBox_stateChanged(int state);

private:
    void createReport();

    QDir chooseDirectoryDialog();

    QString chooseFile();

    void resetUiOptions(const QHash <QString, QVariant> &options);

    void loadSettings();

    void saveSettings();

    const QStringList generateRunnerOptions(const QString &file);

    const QStringList generatePathcerOptions(const QString &file);

    QString executeProcess(const QString &program, const QStringList &options);


    Ui::MainWindow *mUi;
    QDir mTasksDir;
    QString mTasksPath;
    QDir mStudioDir;
    QString mLocalSettings;

    QList <QThread *> mWorkerThreads;
    QList <ModelWorker *> mModelWorkers;

    QList <QPair<QString, QString>> mTasksStatus;

    QHash <QString, QHash <QString, QVariant>> mDirOptions;

    QAtomicInt mActiveModels {0};
};
#endif // MAINWINDOW_H
