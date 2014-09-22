#-------------------------------------------------
#
# Project created by QtCreator 2014-08-15T14:36:31
#
#-------------------------------------------------

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11 -stdlib=libstdc++

INCPATH+=.
INCPATH+=../../include/

DEFINES += _QT

QT       += core gui
qtHaveModule(printsupport): QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gsview
TEMPLATE = app

HEADERS       = \
    muctx.h \
    status.h \
    Cache.h \
    Window.h \
    Document.h \
    Page.h \
    QtUtil.h \
    ScrollingImageList.h \
    ImageWidget.h \
    MyScrollArea.h \
    ThumbnailList.h \
    PageList.h \
    Printer.h

SOURCES       = \
    main.cpp \
    muctx.cpp \
    Cache.cpp \
    Window.cpp \
    Document.cpp \
    Page.cpp \
    QtUtil.cpp \
    ScrollingImageList.cpp \
    ImageWidget.cpp \
    MyScrollArea.cpp \
    ThumbnailList.cpp \
    PageList.cpp \
    Printer.cpp

unix: LIBS += -L$$PWD/../../build/debug/
#  the order of the libraries here is very important.
unix: LIBS += -lmupdf -lfreetype -ljbig2dec -ljpeg -lopenjpeg -lz -lmujs

macx: LIBS += -lssl -lcrypto

INCLUDEPATH += $$PWD/../../build/debug
DEPENDPATH += $$PWD/../../build/debug

unix: PRE_TARGETDEPS += $$PWD/../../build/debug/libfreetype.a
unix: PRE_TARGETDEPS += $$PWD/../../build/debug/libjbig2dec.a
unix: PRE_TARGETDEPS += $$PWD/../../build/debug/libjpeg.a
unix: PRE_TARGETDEPS += $$PWD/../../build/debug/libmujs.a
unix: PRE_TARGETDEPS += $$PWD/../../build/debug/libmupdf.a
unix: PRE_TARGETDEPS += $$PWD/../../build/debug/libopenjpeg.a
unix: PRE_TARGETDEPS += $$PWD/../../build/debug/libz.a

FORMS += \
    Window.ui

RESOURCES += \
    resources.qrc

OTHER_FILES +=
