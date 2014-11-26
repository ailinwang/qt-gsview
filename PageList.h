#ifndef PAGELIST_H
#define PAGELIST_H

#include "ScrollingImageList.h"
#include "Document.h"

class PageList : public ScrollingImageList
{
public:
    PageList();
    bool onEvent(QEvent *e);
    void copyText();
    void deselectText();
    void selectAllText();

    int height() {return getScrollArea()->viewport()->height();}
    int width()  {return getScrollArea()->viewport()->width();}

    void setSearchText (int nPage, std::vector<SearchItem> *items);
    void clearSearchText();
    void hilightSearchText(SearchItem *item);

private:

    void onMousePress(QEvent *e);
    void onMouseRelease(QEvent *e);
    void onMouseMove(QEvent *e);
    void manageCursor(QEvent *e);
    void updateSelection(QEvent *e);
    void onRightClick(QEvent *e);

    void onMenuCopy();
    void onMenuDeselect();
    void onMenuSelectLine();
    void onMenuSelectBlock();
    void onMenuSelectPage();
    void onMenuSelectAll();

    QString collectSelectedText();

    QPoint m_origin;
};

#endif // PAGELIST_H
