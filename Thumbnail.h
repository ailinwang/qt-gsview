#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QPushButton>

#include "muctx.h"

class Thumbnail : public QPushButton
{
    Q_OBJECT
public:
    explicit Thumbnail(QWidget *parent = 0);

    virtual void paintEvent(QPaintEvent *);

    void setPage(int nPage) {m_pageNumber=nPage;}
    void setWindow (void *pWin) {m_window=pWin;}

    bool rendered() const;
    void setRendered(bool rendered);

    double scale() const;
    void setScale(double scale);

    point_t pageSize() const;
    void setPageSize(const point_t &pageSize);

    bool selected() const;
    void setSelected(bool selected);

signals:

public slots:
    void clicked();

private:
    int m_pageNumber = 0;
    void *m_window = NULL;
    bool m_rendered = false;
    double m_scale = 1.0;
    point_t m_pageSize;
    bool m_selected = false;
};

#endif // THUMBNAIL_H
