#ifndef MODELWORKER_H
#define MODELWORKER_H

#include <QObject>

class ModelWorker : public QObject
{
    Q_OBJECT

public:
    ModelWorker(const QString &fileName);

public slots:
    void startExecution(const QString &qrsFile);

signals:
    void finished();

private:
    QString mName;
};

#endif // MODELWORKER_H
