
#include "GSViewApp.h"
#include "Window.h"

#include <QMessageBox>
#include <QtPrintSupport>

int main(int argc, char *argv[])
{
    //  create a GUI app and run it
    GSViewApp app (argc, argv);

//    //  for testing purposes, skip
//    QApplication app(argc, argv);
//    Window *newWindow = new Window();
//    QDesktopWidget dw;
//    int x=dw.width()*0.85;
//    int y=dw.height()*0.85;
//    newWindow->setFixedSize(x,y);
//    newWindow->show();
//    newWindow->OpenFile("/Users/fredross-perry/Desktop/CM District Admin Overview.pdf");

    return app.exec();
}
