#include <QApplication>
#include <QMessageBox>
#include <QtGui>

#include "GSViewApp.h"
#include "Window.h"
#include "QtUtil.h"

GSViewApp::GSViewApp ( int &argc, char **argv ) : QApplication(argc, argv)
{
    QGuiApplication::setApplicationDisplayName(tr("gsview"));

    QEvent *e = new QEvent(QEvent::ApplicationActivated);
    this->postEvent(this, e);
}

bool activatedOnce = false;

bool GSViewApp::event(QEvent *ev)
{
    qDebug() << "app event: " << QtUtil::eventTypeName(ev);

    bool handled = false;
    switch (ev->type())
    {
        case QEvent::FileOpen: {
            m_fileToOpen = static_cast<QFileOpenEvent *>(ev)->file();
//            Window *newWindow = new Window();
//            newWindow->OpenFile(m_fileToOpen);
//            newWindow->show();
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

QString GSViewApp::m_fileToOpen("");

int GSViewApp::exec()
{
    if (!m_fileToOpen.isEmpty())
    {
        //  we were, try and load the file
        Window *newWindow = new Window();
        if (newWindow->OpenFile(m_fileToOpen))
        {
            //  loaded, so show and run
            newWindow->show();
            return QApplication::exec();
        }

        //  not loaded.  Error message and exit.
        delete newWindow;
        return 0;
    }

    //  ask for a new file to open
    Window::open();

    return QApplication::exec();
}
