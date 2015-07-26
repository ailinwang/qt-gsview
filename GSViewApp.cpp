#include <QApplication>
#include <QMessageBox>
#include <QtGui>

#include "GSViewApp.h"
#include "Window.h"
#include "QtUtil.h"

GSViewApp::GSViewApp ( int &argc, char **argv ) : QApplication(argc, argv)
{
    //  these three items are necessary for using the QSettings feature
    QCoreApplication::setOrganizationName(tr("Artifex Software"));
    QCoreApplication::setOrganizationDomain(tr("artifex.com"));
    QCoreApplication::setApplicationName("gsview");

    QGuiApplication::setApplicationDisplayName(tr("gsview"));

    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);

    //  on Linux, we may be asked to open a file on the command line
    if (argc>1)
        m_fileToOpen = argv[1];
}

bool GSViewApp::event(QEvent *ev)
{
    bool handled = false;
    switch (ev->type())
    {
        case QEvent::FileOpen: {
            //  remember the file we should open.
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
    //  start a timer to defer asking for the first file a bit.
    //  this is to give time for a QEvent::FileOpen to be delivered,
    //  if it's going to be.
    QTimer::singleShot(100, gsviewApplication, SLOT(onStarted()));

    return QApplication::exec();
}

void GSViewApp::onStarted()
{
    //  were we asked to load a file?
    if (!m_fileToOpen.isEmpty())
    {
        //  we were, try and load it
        Window *newWindow = new Window();
        newWindow->show();

        if (newWindow->OpenFile(m_fileToOpen))
        {
            //  success
            return;
        }

        //  error
        newWindow->hide();
        delete newWindow;

        //  remove from the recent list.
        QtUtil::removeRecentFile(m_fileToOpen);

        QMessageBox::information(NULL, tr(""), tr("Error opening %1").arg(m_fileToOpen));
    }

    //  ask for a new file to open
    Window::open();
}
