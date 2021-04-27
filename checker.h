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

signals:
	void creatingReport();

	void progressRangeChanged(int minimum, int maximum);

	void progressValueChanged(int value);

	void finished();

	void cancelChecking();

private:
	struct Task {
		QFileInfo qrs;
		const QFileInfoList &fieldsInfos;
		const QStringList &patcherOptions;
		const QStringList &runnerOptions;

//		Task(QFileInfo qrs, const QFileInfoList &fieldsInfos, const QStringList &patcherOptions
//			 , const QStringList &runnerOptions)
//			: qrs(qrs)
//			, fieldsInfos(fieldsInfos)
//			, patcherOptions(patcherOptions)
//			, runnerOptions(runnerOptions)
//		{
//		}

//		Task &operator= (const Task &t) {
//			if (this == &t)
//				return *this;

//			return Task(t.qrs, t.fieldsInfos, t.patcherOptions, t.runnerOptions);
//		}
	};

	struct TaskReport {
		QString name;
		QString task;
		QString error;
		QString time;
	};

	static bool compareReportsByTask(const TaskReport &first, const TaskReport &second)
	{
		return first.task < second.task;
	}

	static QList<TaskReport> checkTask(const Task *task);

	static void reduceFunction(QHash<QString, QList<TaskReport>> &result, const QList<TaskReport> &intermediate);

	static QString startProcess(const QString &program, const QStringList &options);

	void createHtmlReport(QHash<QString, QList<TaskReport>> &result);

	const QStringList generateRunnerOptions(const QHash<QString, QVariant> &options);

	const QStringList generatePathcerOptions(const QHash<QString, QVariant> &options);

	const QString &mTasksPath;
};
