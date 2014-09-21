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

QString eventTypeName(QEvent *event)
{
    static int eventEnumIndex = QEvent::staticMetaObject.indexOfEnumerator("Type");
    QString name = QEvent::staticMetaObject.enumerator(eventEnumIndex).valueToKey(event->type());
    return name;
}

bool ImageWidget::eventFilter (QObject *obj, QEvent *event)
{
    switch( event->type() )
    {
        case QEvent::MouseButtonRelease:
            if (clickable())
            {
                QApplication::postEvent(this->window(), new ImageClickedEvent(m_pageNumber));
            }
            else
            {

            }
            break;

        default:
            break;
    }

    // pass the event on to the parent class
    return QLabel::eventFilter(obj, event);
}
