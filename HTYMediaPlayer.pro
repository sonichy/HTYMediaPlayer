#-------------------------------------------------
#
# Project created by QtCreator 2017-02-13T10:22:17
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYMediaPlayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialogurl.cpp

HEADERS  += mainwindow.h \
    dialogurl.h

FORMS    += mainwindow.ui \
    dialogurl.ui

RESOURCES += \
    filelist.qrc
