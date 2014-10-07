#ifndef PAGELIST_H
#define PAGELIST_H

#include "ScrollingImageList.h"

class PageList : public ScrollingImageList
{
public:
    PageList();

    bool onEvent(QEvent *e);
};

#endif // PAGELIST_H
