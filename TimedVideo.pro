#-------------------------------------------------
#
# Project created by QtCreator 2015-12-08T16:40:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TimedVideo
TEMPLATE = app


SOURCES += main.cpp\
        tvmainwindow.cpp \
    tvgsrecorder.cpp

HEADERS  += tvmainwindow.h \
    tvgsrecorder.h

FORMS    += tvmainwindow.ui
linux* {
    CONFIG += link_pkgconfig
    PKGCONFIG += gstreamer-video-1.0
}

win32 {
    INCLUDEPATH += C:/apps/gstreamer/1.0/x86/include/gstreamer-1.0
    INCLUDEPATH += C:/apps/gstreamer/1.0/x86/lib/gstreamer-1.0/include
    INCLUDEPATH += C:/apps/gstreamer/1.0/x86/include/glib-2.0
    INCLUDEPATH += C:/apps/gstreamer/1.0/x86/lib/glib-2.0/include

    LIBS += -LC:/apps/gstreamer/1.0/x86/lib -lgstvideo-1.0 -lgstbase-1.0 -lgstreamer-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0
}
