#-------------------------------------------------
#
# Project created by QtCreator 2014-08-15T14:36:31
#
#-------------------------------------------------

macx: ICON = resources/gsview_app.icns

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
macx: QMAKE_CXXFLAGS += -stdlib=libstdc++

INCPATH+=.
INCPATH+=../../include/

DEFINES += _QT

macx: DEFINES += _QT_MAC

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
unix: LIBS += -lmupdf -lfreetype -ljbig2dec -ljpeg -lopenjpeg -lz -lmujs -lcups

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

#  The stuff below copies some executable files, and controls where the
#  main app will look for them

unix!macx: {

    OTHER_FILES += \
        unixApps/gs \
        unixApps/gxps

    QMAKE_POST_LINK += $$quote(mkdir -p ./apps $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/unixApps/gs ./apps/gs $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/unixApps/gxps ./apps/gxps $$escape_expand(\n\t))
}

macx: {

    OTHER_FILES += \
        macApps/gs \
        macApps/gxps

    QMAKE_POST_LINK += $$quote(mkdir -p ./apps $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/macApps/gs ./apps/gs $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/macApps/gxps ./apps/gxps $$escape_expand(\n\t))
}


