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
INCPATH+=mupdf/include

#  Linux/release: look for shared libs in ./libs

CONFIG(release,debug|release) {
    unix:!macx {
        QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/libs\''
    }
}

#  turn off warnings
CONFIG += warn_off

#  preprocessor

DEFINES += _QT
macx: DEFINES += _QT_MAC
#  CUPS is not currently working.
#unix: DEFINES += USE_CUPS
win32: DEFINES += _QT_WIN

#  debugging can be easier if we don't use native file dialogs

DEFINES += USE_NATIVE_FILE_DIALOGS=true

#  Qt configuration

QT += core gui
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
    mupdf/source/tools/pdfclean.c \
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
    PrintDialog.ui \
    filedialogextension.ui

RESOURCES += \
    resources.qrc

#  Libraries to link to
#  the order of the libraries here is very important.

LIBS += -L$$PWD/mupdf/build/debug/
unix:  LIBS += -lmupdf -lfreetype -ljbig2dec -ljpeg -lopenjpeg -lz -lmujs
#  not using CUPS
#unix:  LIBS += -lcups
unix:  LIBS += -lssl -lcrypto

#  copy executable files from ghostpdl

unix:!macx {
    QMAKE_POST_LINK += $$quote(mkdir -p $$OUT_PWD/apps $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/ghostpdl/gs/bin/gs $$OUT_PWD/apps/gs $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/ghostpdl/xps/obj/gxps $$OUT_PWD/apps/gxps $$escape_expand(\n\t))
}

macx {
    QMAKE_POST_LINK += $$quote(mkdir -p $$OUT_PWD/gsview.app/Contents/MacOS/apps $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/ghostpdl/gs/bin/gs $$OUT_PWD/gsview.app/Contents/MacOS/apps/gs $$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/ghostpdl/xps/obj/gxps $$OUT_PWD/gsview.app/Contents/MacOS/apps/gxps $$escape_expand(\n\t))
}

#  mac: icon and plist file

macx {
    ICON = resources/gsview_app.icns
    QMAKE_POST_LINK += $$quote(cp $$PWD/resources/gsview_mac.plist $$OUT_PWD/gsview.app/Contents/Info.plist $$escape_expand(\n\t))
}

#  mac:  documentation

macx {
    QMAKE_POST_LINK += $$quote(cp $$PWD/UserGuide.pdf $$OUT_PWD/gsview.app/Contents/Resources/UserGuide.pdf $$escape_expand(\n\t))
}

#  post-link step to get some shared files for the release build

CONFIG(release,debug|release) {
    unix:!macx {

        #  this assumes a relative position of these dirs relative to qmake
        QTLIBPATH    = $$clean_path($$dirname(QMAKE_QMAKE)/..)/lib
        QTPLUGINPATH = $$clean_path($$dirname(QMAKE_QMAKE)/..)/plugins/platforms

        QMAKE_POST_LINK += $$quote(rm -rf ./libs $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(mkdir -p ./libs $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(rm -rf ./platforms $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(mkdir -p ./platforms $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTLIBPATH/libQt5PrintSupport.so.5 ./libs/libQt5PrintSupport.so.5 $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTLIBPATH/libQt5Widgets.so.5 ./libs/libQt5Widgets.so.5 $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTLIBPATH/libQt5Gui.so.5 ./libs/libQt5Gui.so.5 $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTLIBPATH/libQt5Core.so.5 ./libs/libQt5Core.so.5 $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTLIBPATH/libQt5DBus.so.5 ./libs/libQt5DBus.so.5 $$escape_expand(\n\t))
        QMAKE_POST_LINK += $$quote(cp $$QTPLUGINPATH/libqxcb.so ./platforms/libqxcb.so $$escape_expand(\n\t))
    }
}

DISTFILES += \
    UserGuide.pdf
