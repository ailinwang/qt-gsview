#-------------------------------------------------
#
# Project created by QtCreator 2014-08-15T14:36:31
#
#-------------------------------------------------

macx: ICON = resources/gsview_app.icns

#  C and C++ compiler configuration
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
macx: QMAKE_CXXFLAGS += -stdlib=libstdc++

#  include file paths
INCPATH+=.
INCPATH+=../../include/

#  proprocessor
DEFINES += _QT
macx: DEFINES += _QT_MAC
unix: DEFINES += USE_CUPS

#  Qt configuration
QT       += core gui
qtHaveModule(printsupport): QT += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = gsview
TEMPLATE = app

#  various files
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
    Printer.h \
    AboutDialog.h \
    FileInfoDialog.h

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
    Printer.cpp \
    AboutDialog.cpp \
    FileInfoDialog.cpp

FORMS += \
    Window.ui \
    AboutDialog.ui \
    FileInfoDialog.ui

RESOURCES += \
    resources.qrc

#  Libraries to link to
#  the order of the libraries here is very important.
LIBS += -L$$PWD/../../build/debug/
unix:  LIBS += -lmupdf -lfreetype -ljbig2dec -ljpeg -lopenjpeg -lz -lmujs -lcups
win32: LIBS += -lmupdf -lfreetype -ljbig2dec -ljpeg -lopenjpeg -lz -lmujs
macx:  LIBS += -lssl -lcrypto

#  The stuff below copies some executable files, and controls where the
#  main app will look for them

unix:!macx {
    OTHER_FILES += \
        linuxApps/gs \
        linuxApps/gxps

    QMAKE_POST_LINK += $$quote(mkdir -p ./apps $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/linuxApps/gs ./apps/gs $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/linuxApps/gxps ./apps/gxps $$escape_expand(\n\t))
}

macx {
    OTHER_FILES += \
        macApps/gs \
        macApps/gxps
    QMAKE_POST_LINK += $$quote(mkdir -p ./apps $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/macApps/gs ./apps/gs $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/macApps/gxps ./apps/gxps $$escape_expand(\n\t))
}
