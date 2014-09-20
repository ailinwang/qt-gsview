#include "QtUtil.h"

#include <QMessageBox>

QImage * QtUtil::QImageFromData(unsigned char *samples, int w, int h)
{
    QImage *myImage = new QImage(w, h, QImage::Format_ARGB32);

    int index = 0;
    for (int row=0; row <h ;row++)
    {
        for (int col=0; col <w ;col++)
        {
            //  image comes in as RGBA
            unsigned char red   = samples[index];
            unsigned char green = samples[index+1];
            unsigned char blue  = samples[index+2];
            unsigned char alpha = samples[index+3];
            index+= 4;
            uint pixel = red + (green<<8) + (blue<<16) + (alpha<<24);

            myImage->setPixel (col, row, pixel);
        }
    }

    return myImage;
}

void QtUtil::errorMessage(const std::string theTitle, const std::string theMessage)
{
    QMessageBox::critical(NULL, QString(theTitle.c_str()), QString(theMessage.c_str()));
}
