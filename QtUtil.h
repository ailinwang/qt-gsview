#ifndef QTUTIL_H
#define QTUTIL_H

//// use expression as sub-expression,
//// then make type of full expression int, discard result
//#define UNUSED(x) (void)(sizeof((x), 0))

#include <QImage>

class QtUtil
{
public:
    static QString getAppsPath();
    static QString getRealAppDirPath();
    static QString getTempFolderPath();
    static QString getGsPath();
    static QString getGxpsPath();
    static QString eventTypeName(QEvent *event);
    static QString extensionFromPath(QString path);
    static QString extensionFromFilter(QString filter);
    static QRect mapToGlobal ( QWidget*widget, QRect r);
    static QRect mapFromGlobal ( QWidget*widget, QRect r);

    static QStringList getRecentFileList();
    static void setRecentFileList (QStringList list);
    static void addRecentFile(QString path);
    static void removeRecentFile(QString path);
    static const int maxRecentFiles = 10;
    static QString getLastOpenFileDir();
    static void setLastOpenFileDir(QString val);
};

#endif // QTUTIL_H
