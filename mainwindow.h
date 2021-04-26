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

	void on_wPcheckBox_stateChanged(int state);

	void on_wPPCheckBox_stateChanged(int state);

	void on_resetPCheckBox_stateChanged(int state);

	void on_showConsoleCheckBox_stateChanged(int state);

private:
	struct TaskReport {
		QString name;
		QString task;
		QString error;
		QString time;
	};

	void createReport(const QList<MainWindow::TaskReport> &result);

	static QList<TaskReport> runCheck(const QFileInfo &name, const QFileInfoList &fieldsInfos, const QStringList &patcherOptions, const QStringList &modelOptions);

	struct Task {
		QFileInfo qrs;
		QFileInfoList fieldsInfos;
		QStringList patcherOptions;
		QStringList modelOptions;
	};

	static QList<TaskReport> runCheckFromTask(const Task &task);

	static QList<TaskReport> runAllChecks(const QFileInfoList &tasksInfos, const QFileInfoList &fieldsInfos
										  , QStringList patcherOptions, QStringList modelOptions);

	static void reduceFunction(QList<MainWindow::TaskReport> &result, const QList<MainWindow::TaskReport> &intermediate);

	QDir chooseDirectoryDialog();

	void resetUiOptions(const QHash <QString, QVariant> &options);

	void loadSettings();

	void saveSettings();

	const QStringList generateRunnerOptions(const QString &file);

	const QStringList generatePathcerOptions(const QString &file);

	static QString executeProcess(const QString &program, const QStringList &options);


	Ui::MainWindow *mUi;
	QDir mTasksDir;
	QDir mFieldsDir;
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
