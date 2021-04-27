#include <QString>
#include <QVariant>

#pragma once

const QString closeSuccessOption = "closeOnSuccess";
const QString backgroundOption = "background";
const QString consoleOption = "showConsole";

const QString xmlFieldsDir = "fieldsDir";
const QString patchField = "patchField";
const QString patchWorld = "patchWorld";
const QString patchWP = "patchWroldAndPosition";
const QString resetRP = "resetRobotPosition";

const QHash <QString, QVariant> defaultOptions {{closeSuccessOption, true}
											   ,{backgroundOption, false}
											   ,{consoleOption, false}
											   ,{xmlFieldsDir, ""}
											   ,{patchField, true}
											   ,{patchWP, false}};

