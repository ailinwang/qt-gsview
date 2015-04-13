#ifndef PAGELIST_H
#define PAGELIST_H

#include "ScrollingImageList.h"
#include "Document.h"

class SelectionFrame;
class Window;
class FileSave;

class PageList : public ScrollingImageList
{
public:
    PageList(Window *parent);
    bool onEvent(QEvent *e);
    void copyText();
    void deselectText();
    void selectAllText();

    int height() {return getScrollArea()->viewport()->height();}
    int width()  {return getScrollArea()->viewport()->width();}

    void setSearchText (int nPage, std::vector<SearchItem> *items);
    void clearSearchText();
    void hilightSearchText(SearchItem *item);

    bool isAreaSelected();
    void saveSelection(FileSave *fileSave);
    virtual void zoom (double scale, bool resizing);
    virtual void onScrollChange();
    virtual void onSliderReleased();
    virtual void onMouseEnter();
    virtual void onMouseLeave();

public slots:

    virtual void onScrollingTimer();

private:

    void onMousePress(QEvent *e);
    void onMouseRelease(QEvent *e);
    void onMouseMove(QEvent *e);
    void manageCursor(QEvent *e);
    void updateSelection(QEvent *e);
    void updateSelection(QPoint p);
    void onRightClick(QEvent *e);
    void onRightClickArea(QEvent *e);

    void onMenuCopy();
    void onMenuDeselect();
    void onMenuSelectLine(QEvent *e);
    void onMenuSelectBlock(QEvent *e);
    void onMenuSelectPage();
    void onMenuSelectAll();

    QString collectSelectedText();

    int charIndex(TextLine *line, ImageWidget *widget, QPoint pos);
    QPoint mapToContent(QWidget *w, QPoint p);

    QPoint m_origin;

    SelectionFrame *m_rubberBand=NULL;
    bool m_selectingArea = false;
    QPoint m_rubberBandOrigin;
    bool m_controlKeyIsDown = false;
    double m_rubberbandScale=1.0;
    int m_rubberbandpage=0;
    QRect m_rubberbandRect;

    Window *m_window=NULL;

    QTimer *m_scrollingTimer= NULL;
    QPoint m_leftAt;
    void autoScroll();
};

#endif // PAGELIST_H
