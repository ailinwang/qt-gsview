#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QLabel>
#include <QEvent>

#include "muctx.h"

//  custom event that's sent when we are clicked
class ImageClickedEvent : public QEvent
{
public:
    ImageClickedEvent (const int pageNumber): QEvent(IMAGE_CLICKED_EVENT), m_pageNumber(pageNumber) {}
    int getPageNumber() const {return m_pageNumber;}
    static const QEvent::Type IMAGE_CLICKED_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);
private:
    int m_pageNumber;
};

class ImageWidget : public QLabel
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = 0);

    virtual void paintEvent(QPaintEvent *);

    void setPage(int nPage) {m_pageNumber=nPage;}
    void setWindow (void *pWin) {m_window=pWin;}

    bool rendered() const {return m_rendered;}
    void setRendered(bool rendered) {m_rendered = rendered;}

    double scale() const {return m_scale;}
    void setScale(double scale) {m_scale = scale;}

    point_t pageSize() const {return m_pageSize;}
    void setPageSize(const point_t &pageSize){m_pageSize = pageSize;}

    bool selected() const {return m_selected;}
    void setSelected(bool selected);

    bool clickable() const {return m_clickable;}
    void setClickable(bool val) {m_clickable = val;}

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    int m_pageNumber = 0;
    void *m_window = NULL;
    bool m_rendered = false;
    double m_scale = 1.0;
    point_t m_pageSize;
    bool m_selected = false;
    bool m_clickable = false;
};

#endif // IMAGEWIDGET_H
