#ifndef PAGELIST_H
#define PAGELIST_H

#include "ScrollingImageList.h"

class PageList : public ScrollingImageList
{
public:
    PageList();
    bool onEvent(QEvent *e);
    void copyText();
    void deselectText();
    void selectAllText();

private:

    void onMousePress(QEvent *e);
    void onMouseRelease(QEvent *e);
    void onMouseMove(QEvent *e);
    void manageCursor(QEvent *e);
    void updateSelection(QEvent *e);

    QPoint m_origin;
};

#endif // PAGELIST_H
