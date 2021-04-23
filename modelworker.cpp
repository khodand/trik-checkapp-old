#include "modelworker.h"

#include <QProcess>

ModelWorker::ModelWorker(const QString &fileName)
    : mName(fileName)
{
}

void ModelWorker::startExecution(const QString &qrsFile)
{
    QProcess::startDetached("2D-model", {qrsFile, "--close-on-succes"});
    emit finished();
}
