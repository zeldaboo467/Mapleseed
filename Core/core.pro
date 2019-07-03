#-------------------------------------------------
#
# Project created by QtCreator 2019-06-23T18:16:20
#
#-------------------------------------------------

QT += core gui xml network concurrent gamepad

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MapleSeed
TEMPLATE = app

RC_FILE = resource.rc

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        helper.cpp \
        logging.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        helper.h \
        logging.h \
        mainwindow.h \
        settings.h \
        titleinfo.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += C:/OpenSSL-v111-Win64/include
DEPENDPATH += C:/OpenSSL-v111-Win64/include

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../CemuDB/release/ -lCemuDatabase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../CemuDB/debug/ -lCemuDatabase
else:unix: LIBS += -L$$OUT_PWD/../CemuDB/ -lCemuDatabase

INCLUDEPATH += $$PWD/../CemuDB
DEPENDPATH += $$PWD/../CemuDB

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../CemuLibrary/release/ -lCemuLibrary
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../CemuLibrary/debug/ -lCemuLibrary
else:unix: LIBS += -L$$OUT_PWD/../CemuLibrary/ -lCemuLibrary

INCLUDEPATH += $$PWD/../CemuLibrary
DEPENDPATH += $$PWD/../CemuLibrary

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Network/release/ -lNetwork
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Network/debug/ -lNetwork
else:unix: LIBS += -L$$OUT_PWD/../Network/ -lNetwork

INCLUDEPATH += $$PWD/../Network
DEPENDPATH += $$PWD/../Network

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../CemuCrypto/release/ -lCemuCrypto
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../CemuCrypto/debug/ -lCemuCrypto
else:unix: LIBS += -L$$OUT_PWD/../CemuCrypto/ -lCemuCrypto

INCLUDEPATH += $$PWD/../CemuCrypto
DEPENDPATH += $$PWD/../CemuCrypto
