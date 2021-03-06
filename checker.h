#pragma once

#include <QFileInfo>
#include <QString>

class Checker : public QObject
{
	Q_OBJECT

public:
	Checker(const QString &tasksPath);

	void revieweTasks(const QFileInfoList &qrsInfos, const QFileInfoList &fieldsInfos, const QHash<QString
					  , QVariant> &options);
	struct Task {
		QFileInfo qrs;
		const QFileInfoList &fieldsInfos;
		const QStringList &patcherOptions;
		const QStringList &runnerOptions;
	};

	struct TaskReport {
		QString name;
		QString task;
		QString error;
		QString time;
	};

private:
	static bool compareReportsByTask(const TaskReport &first, const TaskReport &second)
	{
		return first.task < second.task;
	}

	static void reduceFunction(QHash<QString, QList<TaskReport>> &result, const QList<TaskReport> &intermediate);

	static QList<TaskReport> checkTask(const Task *task);

	static QString startProcess(const QString &program, const QStringList &options);

	void createHtmlReport(QHash<QString, QList<TaskReport>> &result);

	const QStringList generateRunnerOptions(const QHash<QString, QVariant> &options);

	const QStringList generatePathcerOptions(const QHash<QString, QVariant> &options);

	static bool isErrorMessage(const QString &message);

	const QString &mTasksPath;
};
