#include "QtUtil.h"

#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QtWidgets>
#include <QEvent>


QImage * QtUtil::QImageFromData(unsigned char *samples, int w, int h)
{
    //  An earlier version of the code used to rearrange the pixels
    //  to match the expected image format (commented-out below).
    //  this no longer seems to be necessary, so we let
    //  QImage create itself directly from the pixels given.

    QImage *myImage = new QImage(samples, w, h, QImage::Format_ARGB32);

//    QImage *myImage = new QImage(w, h, QImage::Format_ARGB32);
//
//    int index = 0;
//    for (int row=0; row <h ;row++)
//    {
//        for (int col=0; col <w ;col++)
//        {
//            //  image comes in as RGBA
//            unsigned char red   = samples[index];
//            unsigned char green = samples[index+1];
//            unsigned char blue  = samples[index+2];
//            unsigned char alpha = samples[index+3];
//            index+= 4;
//            uint pixel = red + (green<<8) + (blue<<16) + (alpha<<24);
//
//            myImage->setPixel (col, row, pixel);
//        }
//    }

    return myImage;
}

QString getRealAppDirPath()
{
    QString path = qApp->applicationDirPath();
    if (!path.endsWith( QDir::separator()))
        path +=  QDir::separator();

#ifdef _QT_MAC
    path += QString("../../../");
#endif

    return path;
}

QString QtUtil::getGsPath()
{
#ifdef  _QT_WIN
    return getRealAppDirPath() + QString("apps/gs/gswin32c.exe");
#endif
    return getRealAppDirPath() + QString("apps/gs");
}

QString QtUtil::getGxpsPath()
{
#ifdef  _QT_WIN
    return getRealAppDirPath() + QString("apps/gxps/gxps.exe");
#endif
    return getRealAppDirPath() + QString("apps/gxps");
}

//  temp folder stuff
static bool tempDone = false;
static QString tempFolderPath("");

QString QtUtil::getTempFolderPath()
{
    //  create the temp path
    if (!tempDone)
    {
        tempDone = true;

        tempFolderPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        tempFolderPath += "/";
        tempFolderPath += "gsview/";

//        qDebug() << "Temp path is" << tempFolderPath;

        //  create the folder.
        QDir dir(tempFolderPath);
        if (!dir.exists())
            dir.mkpath(".");
    }

    return tempFolderPath;
}

QString QtUtil::eventTypeName(QEvent *event)
{
    static int eventEnumIndex = QEvent::staticMetaObject.indexOfEnumerator("Type");
    QString name = QEvent::staticMetaObject.enumerator(eventEnumIndex).valueToKey(event->type());
    return name;
}

QString QtUtil::extensionFromPath(QString path)
{
    QFileInfo fileInfo (path);
    return fileInfo.suffix().toLower();
}

QString QtUtil::extensionFromFilter(QString filter)
{
    int start = 0;
    int end = filter.length()-1;

    for (int i=0;i<filter.length();i++)
    {
        if (filter.at(i) == QChar('*'))
            start = i+2;

        if (filter.at(i) == QChar(')'))
        {
            end = i-1;
            break;
        }
    }

    return filter.mid(start,end-start+1).toLower();
}




