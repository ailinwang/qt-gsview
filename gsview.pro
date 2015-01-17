#-------------------------------------------------
#
# Project created by QtCreator 2014-08-15T14:36:31
#
#-------------------------------------------------


#  C and C++ compiler configuration
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
macx: QMAKE_CXXFLAGS += -stdlib=libstdc++

#  include file paths
INCPATH+=.
INCPATH+=../../include/

#  look for shared libs in ./libs
#  unix only, release only
CONFIG(release,debug|release) {
    unix:!macx {
        QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/libs\'"
    }
}

#  proprocessor
DEFINES += _QT
macx: DEFINES += _QT_MAC
unix: DEFINES += USE_CUPS
win32: DEFINES += _QT_WIN

#  debugging can be easier if we don't use native file dialogs
DEFINES += USE_NATIVE_FILE_DIALOGS=true

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
    QtUtil.h \
    ScrollingImageList.h \
    ImageWidget.h \
    ThumbnailList.h \
    PageList.h \
    Printer.h \
    AboutDialog.h \
    FileInfoDialog.h \
    ContentsList.h \
    FileSave.h \
    MessagesDialog.h \
    GSViewApp.h \
    FileSaveDialog.h \
    ExtractPagesDialog.h \
    ICCDialog.h \
    ICCDialog2.h \
    SelectionFrame.h \
    PrintDialog.h

SOURCES       = \
    main.cpp \
    muctx.cpp \
    Cache.cpp \
    Window.cpp \
    Document.cpp \
    QtUtil.cpp \
    ScrollingImageList.cpp \
    ImageWidget.cpp \
    ThumbnailList.cpp \
    PageList.cpp \
    Printer.cpp \
    AboutDialog.cpp \
    FileInfoDialog.cpp \
    ContentsList.cpp \
    FileSave.cpp \
    ../../source/tools/pdfclean.c \
    MessagesDialog.cpp \
    GSViewApp.cpp \
    FileSaveDialog.cpp \
    ExtractPagesDialog.cpp \
    ICCDialog.cpp \
    ICCDialog2.cpp \
    SelectionFrame.cpp \
    PrintDialog.cpp

FORMS += \
    Window.ui \
    AboutDialog.ui \
    FileInfoDialog.ui \
    MessagesDialog.ui \
    ExtractPagesDialog.ui \
    ICCDialog.ui \
    ICCDialog2.ui \
    PrintDialog.ui

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

#win32 {
#    OTHER_FILES += \
#        winApps/gs \
#        winApps/gxps
#    QMAKE_POST_LINK += $$quote(rmdir /S /Q apps $$escape_expand(\n\t))
#    QMAKE_POST_LINK += $$quote(mkdir apps $$escape_expand(\n\t))
#    QMAKE_POST_LINK += $$quote(xcopy $$PWD/winApps/gs apps/gs $$escape_expand(\n\t))
#    QMAKE_POST_LINK += $$quote(xcopy $$PWD/winApps/gxps apps/gxps $$escape_expand(\n\t))
#}

macx {
    OTHER_FILES += resources/gsview_app.icns
    ICON = resources/gsview_app.icns
    OTHER_FILES += resources/gsview_mac.plist
    QMAKE_INFO_PLIST = '''resources/gsview_mac.plist'''
}

#  post-link step to get some shared files for the release build

QTLIBPATH = /home/fred/Qt5.3.2/5.3/gcc_64/lib
CONFIG(release,debug|release) {
    unix:!macx {
        QMAKE_POST_LINK += $$quote(rmdir --ignore-fail-on-non-empty ./libs $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(mkdir -p ./libs $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTLIBPATH/libQt5PrintSupport.so.5.3.2 ./libs/libQt5PrintSupport.so.5 $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTLIBPATH/libQt5Widgets.so.5.3.2 ./libs/libQt5Widgets.so.5 $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTLIBPATH/libQt5Gui.so.5.3.2 ./libs/libQt5Gui.so.5 $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTLIBPATH/libQt5Core.so.5.3.2 ./libs/libQt5Core.so.5 $$escape_expand(\n\t))
    }
}

