#ifndef THUMBNAILLIST_H
#define THUMBNAILLIST_H

#include "ScrollingImageList.h"

class ThumbnailList : public ScrollingImageList
{
public:
    ThumbnailList();

    virtual double getScale();

protected:
};

#endif // THUMBNAILLIST_H
