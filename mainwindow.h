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
		QString error;
		QString time;
	};

	void createReport();

	static TaskReport runCheck(const QString &name, const QStringList &patcherOptions, const QStringList &modelOptions);

	static QList<TaskReport> runAllChecks(const QStringList &tasks
										  , QStringList patcherOptions, QStringList modelOptions);

	QDir chooseDirectoryDialog();

	void resetUiOptions(const QHash <QString, QVariant> &options);

	void loadSettings();

	void saveSettings();

	const QStringList generateRunnerOptions(const QString &file);

	const QStringList generatePathcerOptions(const QString &file);

	static QString executeProcess(const QString &program, const QStringList &options);


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
