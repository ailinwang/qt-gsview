#ifndef QTUTIL_H
#define QTUTIL_H

#include <QImage>

class QtUtil
{
public:
    static QImage * QImageFromData(unsigned char *samples, int w, int h);
    static void errorMessage(const std::string theTitle, const std::string theMessage);

    static QString getTempFolderPath();
    static QString getGsPath();
    static QString getGxpsPath();
};

#endif // QTUTIL_H
