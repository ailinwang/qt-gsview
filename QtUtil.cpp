#include "QtUtil.h"

#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QtWidgets>
#include <QEvent>


QString getRealAppDirPath()
{
    QString path = qApp->applicationDirPath();
    if (!path.endsWith( QDir::separator()))
        path +=  QDir::separator();

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

QRect QtUtil::mapToGlobal ( QWidget *widget, QRect r)
{
    QPoint tl = widget->mapToGlobal(r.topLeft());
    QPoint br = widget->mapToGlobal(r.bottomRight());
    QRect r2(tl, br);
    return r2;
}

QRect QtUtil::mapFromGlobal ( QWidget*widget, QRect r)
{
    QPoint tl = widget->mapFromGlobal(r.topLeft());
    QPoint br = widget->mapFromGlobal(r.bottomRight());
    QRect r2(tl, br);
    return r2;
}
