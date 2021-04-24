QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    modelworker.cpp

HEADERS += \
    mainwindow.h \
    modelworker.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    trikCheckApp_ru_RU.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
	QMAKE_MANIFEST = $$PWD/application.exe.manifest
	RC_ICONS = icon.ico
	DISTFILES += $$QMAKE_MANIFEST $$RC_ICONS
}

macx {
	ICON = icon.icns
	QMAKE_INFO_PLIST = mainWindow.plist
}

