#include <QtWidgets>

#include "Thumbnail.h"
#include "Window.h"

Thumbnail::Thumbnail(QWidget *parent) :
    QPushButton(parent)
{
    connect(this, SIGNAL(clicked()), this, SLOT(clicked()));
}

void Thumbnail::paintEvent(QPaintEvent *event)
{
    //  draw ourselves
    QPushButton::paintEvent(event);

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

void Thumbnail::clicked()
{
    //  this needs to be Window-agnostic.
    //  can we send messages without knowing the receiver?
    Window *w = (Window *) this->window();
    w->clickedThumb(m_pageNumber);
}

bool Thumbnail::selected() const
{
    return m_selected;
}

void Thumbnail::setSelected(bool isSelected)
{
    if (isSelected != m_selected)
    {
        m_selected = isSelected;
        repaint();
    }

    m_selected = isSelected;
}

point_t Thumbnail::pageSize() const
{
    return m_pageSize;
}

void Thumbnail::setPageSize(const point_t &pageSize)
{
    m_pageSize = pageSize;
}

double Thumbnail::scale() const
{
    return m_scale;
}

void Thumbnail::setScale(double scale)
{
    m_scale = scale;
}

bool Thumbnail::rendered() const
{
    return m_rendered;
}

void Thumbnail::setRendered(bool rendered)
{
    m_rendered = rendered;
}

