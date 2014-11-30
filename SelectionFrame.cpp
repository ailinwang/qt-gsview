#include <QPainter>
#include <QRubberBand>
#include <QtWidgets>

#include "SelectionFrame.h"

SelectionFrame::SelectionFrame(QWidget *parent) :
    QRubberBand(QRubberBand::Rectangle, parent)
{
}

void SelectionFrame::paintEvent(QPaintEvent *pe)
{
    QPainter painter;
    QPen pen(Qt::red, 4);
    pen.setStyle(Qt::SolidLine);

    painter.begin(this);
    painter.setClipping(false);
    painter.setPen(pen);
    painter.drawRect(pe->rect());
    painter.end();
}
