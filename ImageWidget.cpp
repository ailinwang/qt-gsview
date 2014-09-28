#include <QtWidgets>
#include <QEvent>

#include "ImageWidget.h"
#include "Window.h"

ImageWidget::ImageWidget(QWidget *parent) :
    QLabel(parent)
{
    this->window()->installEventFilter(this);
}

void ImageWidget::paintEvent(QPaintEvent *event)
{
    //  draw ourselves
    QLabel::paintEvent(event);

    //  draw 'selected" appearance
    if (selected())
    {
        QPainter painter(this);

        QRect rect(0,0,this->width(),this->height());
        rect.translate(2,2);
        rect.setWidth(rect.width()-4);
        rect.setHeight(rect.height()-4);

        QRectF rectf(rect);
        QPen lineStyle (QColor("#24A719"), 2);
        painter.setPen(lineStyle);
        painter.drawRect(rectf);
    }
}

void ImageWidget::setSelected(bool isSelected)
{
    if (isSelected != m_selected)
    {
        m_selected = isSelected;
        repaint();
    }

    m_selected = isSelected;
}

bool ImageWidget::eventFilter (QObject *obj, QEvent *event)
{
    //  process the event
    bool result = QLabel::eventFilter(obj, event);

    //  post event if it was a click
    if (event->type() == QEvent::MouseButtonRelease)
        QApplication::postEvent(this->window(), new ImageClickedEvent(m_pageNumber));

    //  done
    return result;
}

