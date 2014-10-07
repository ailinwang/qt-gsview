#ifndef PAGELIST_H
#define PAGELIST_H

#include "ScrollingImageList.h"

class PageList : public ScrollingImageList
{
public:
    PageList();
    bool onEvent(QEvent *e);

private:

    void onMousePress(QEvent *e);
    void onMouseRelease(QEvent *e);
    void onMouseMove(QEvent *e);
    void manageCursor(QEvent *e);
    void updateSelection(QEvent *e);

//    void renderSelection();

    QPoint m_origin;
//    std::vector<TextLine> m_selected_lines;
};

#endif // PAGELIST_H
