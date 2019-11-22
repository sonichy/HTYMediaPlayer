QT       += core gui multimedia multimediawidgets xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYMP
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialogurl.cpp

HEADERS  += mainwindow.h \
    dialogurl.h

FORMS    += mainwindow.ui \
    dialogurl.ui

RESOURCES += res.qrc

RC_FILE += res.rc