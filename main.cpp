
#include "GSViewApp.h"
#include "Window.h"

#include <QMessageBox>
#include <QtPrintSupport>

int main(int argc, char *argv[])
{
#if 1

    //  create a GUI app and run it
    GSViewApp app (argc, argv);

#ifdef _QT_MAC
    //  don't quit when the last file is closed.
    //  this only works for OS X, which has a menu bar
    app.setQuitOnLastWindowClosed(false);
#endif

    return app.exec();

#else

    //  for testing purposes
    QApplication app(argc, argv);
    Window *newWindow = new Window();
    newWindow->show();
    newWindow->OpenFile("/Users/fredross-perry/Desktop/CM District Admin Overview.pdf");
//        newWindow->OpenFile("/home/fred/Desktop/sample files/pdf/CM District Admin Overview.pdf");
    return app.exec();

#endif

}
