QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYMP
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialogurl.cpp \
    waveform.cpp

HEADERS  += mainwindow.h \
    dialogurl.h \
    waveform.h

FORMS    += mainwindow.ui \
    dialogurl.ui

RESOURCES += res.qrc

RC_FILE += res.rc
