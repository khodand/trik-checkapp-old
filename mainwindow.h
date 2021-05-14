#pragma once

#include <QDir>
#include <QMainWindow>

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

	void on_closeOnSuccessOption_stateChanged(int arg1);

private:
	QDir chooseDirectoryDialog();

	void resetUiOptions(const QHash <QString, QVariant> &options);

	void loadSettings();

	void saveSettings();

	Ui::MainWindow *mUi;
	QDir mTasksDir;
	QDir mFieldsDir;
	QString mTasksPath;
	QDir mStudioDir;
	QString mLocalSettings;

	QHash <QString, QHash <QString, QVariant>> mDirOptions;
};
