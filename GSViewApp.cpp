#include <QApplication>
#include <QMessageBox>
#include <QtGui>

#include "GSViewApp.h"
#include "Window.h"
#include "QtUtil.h"

GSViewApp::GSViewApp ( int &argc, char **argv ) : QApplication(argc, argv)
{
    QGuiApplication::setApplicationDisplayName(tr("gsview"));
}

bool GSViewApp::event(QEvent *ev)
{
    qDebug() << "app event: " << QtUtil::eventTypeName(ev);

    bool handled = false;
    switch (ev->type())
    {
        case QEvent::FileOpen: {
            m_fileToOpen = static_cast<QFileOpenEvent *>(ev)->file();
            handled = true;
            break;
        }

        default: {
            handled = QApplication::event(ev);
            break;
        }
    }

    return handled;
}

int GSViewApp::exec()
{
    QTimer::singleShot(100, gsviewApplication, SLOT(onStarted()));

    return QApplication::exec();
}

void GSViewApp::onStarted()
{
    if (!m_fileToOpen.isEmpty())
    {
        //  we were, try and load the file
        Window *newWindow = new Window();
        if (newWindow->OpenFile(m_fileToOpen))
        {
            //  loaded, so show and run
            newWindow->show();
            return;
        }

        //  not loaded.  Error message and exit.
        delete newWindow;
        exit();
    }

    //  ask for a new file to open
    Window::open();
}
