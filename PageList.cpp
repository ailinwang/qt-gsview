#include <QPoint>
#include <QRubberBand>
#include <QMouseEvent>

#include "PageList.h"

PageList::PageList()
{
}

QPoint origin;
QRubberBand *rubberBand = NULL;

bool PageList::onEvent(QEvent *e)
{
    if (e->type() == QMouseEvent::MouseButtonPress)
    {
        origin = ((QMouseEvent *)e)->pos();
        if (!rubberBand)
            rubberBand = new QRubberBand(QRubberBand::Rectangle, getScrollArea());
        rubberBand->setGeometry(QRect(origin, QSize()));
        rubberBand->show();
    }

    else if (e->type() == QMouseEvent::MouseButtonRelease)
    {
        rubberBand->hide();
    }

    else if (e->type() == QMouseEvent::MouseMove)
    {
        if(((QMouseEvent *)e)->buttons() == Qt::LeftButton)
        {
            rubberBand->setGeometry(QRect(origin, ((QMouseEvent *)e)->pos()).normalized());
        }
    }

    return false;
}
