
#include "GSViewApp.h"
#include "Window.h"

#include <QMessageBox>

int main(int argc, char *argv[])
{
    //  create a GUI app
    GSViewApp app (argc, argv);

//    //  see if we were launched with a command line parameter
//    QCommandLineParser commandLineParser;
//    commandLineParser.addHelpOption();
//    commandLineParser.addPositionalArgument(Window::tr("[file]"), Window::tr("File to open."));
//    commandLineParser.process(QCoreApplication::arguments());
//    if (!commandLineParser.positionalArguments().isEmpty())
//    {
//        QString path = commandLineParser.positionalArguments().front();
//        QMessageBox::information(NULL, Window::tr(""), path);
//    }

    //  run the app
    return app.exec();
}
