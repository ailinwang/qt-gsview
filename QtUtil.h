#ifndef QTUTIL_H
#define QTUTIL_H

#include <qimage.h>

class QtUtil
{
public:
    static QImage * QImageFromData(unsigned char *samples, int w, int h);
};

#endif // QTUTIL_H
