#-------------------------------------------------
#
# Project created by QtCreator 2015-03-02T14:20:32
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QMikAPI
TEMPLATE = app


SOURCES += main.cpp\
    QSentences.cpp \
    QMD5.cpp \
    QIniFile.cpp \
    Comm.cpp \
    QMikAPIExample.cpp

HEADERS  += \
    QSentences.h \
    QMD5.h \
    QIniFile.h \
    Comm.h \
    QMikAPIExample.h

FORMS    += \
    QMikAPIExample.ui

DISTFILES += \
    README.md \
    COPYING.LESSER \
    COPYING \
    README.txt
