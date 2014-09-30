#include <QtWidgets>
#include <QEvent>

#include "ImageWidget.h"
#include "Window.h"
#include "Document.h"

ImageWidget::ImageWidget(QWidget *parent) :
    QLabel(parent)
{
    setMouseTracking(true);
    this->window()->installEventFilter(this);
}

void ImageWidget::paintEvent(QPaintEvent *event)
{
    //  draw ourselves
    QLabel::paintEvent(event);

    QPainter painter(this);

    //  draw 'selected" appearance
    if (selected())
    {
        QRect rect(0,0,this->width(),this->height());
        rect.translate(2,2);
        rect.setWidth(rect.width()-4);
        rect.setHeight(rect.height()-4);
        QRectF rectf(rect);

        QPen lineStyle (QColor("#24A719"), 2);
        painter.setPen(lineStyle);
        painter.drawRect(rectf);
    }

    if (showLinks())
    {
        int n = m_document->ComputeLinks(m_pageNumber);
        for (int i=0;i<n;i++)
        {
            Link *link = m_document->GetLink(m_pageNumber, i);
            if (link != NULL)
            {
                //  make sure the rect is scaled
                QRect rect(QPoint(m_scale*link->left,m_scale*link->top), QPoint(m_scale*link->right,m_scale*link->bottom));
                QRectF rectf(rect);
                QPen lineStyle (QColor("#24A719"), 2);
                painter.setPen(lineStyle);
                painter.drawRect(rectf);
            }
        }
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
    {
        if (m_inLink && m_link)
        {
            //  launch the url
            QUrl url(QString(m_link->Uri.c_str()));
            bool result = QDesktopServices::openUrl(url);
        }
        else
        {
            QApplication::postEvent(this->window(), new ImageClickedEvent(m_pageNumber));
        }
    }

    //  done
    return result;
}

void ImageWidget::mouseMoveEvent( QMouseEvent * event )
{
    int x = event->x();
    int y = event->y();

    bool inLink = false;

    int n = m_document->ComputeLinks(m_pageNumber);
    Link *linkIAmIn = NULL;
    for (int i=0;i<n;i++)
    {
        Link *link = m_document->GetLink(m_pageNumber, i);
        if (link != NULL)
        {
            //  make sure the rect is scaled
            QRect rect(QPoint(m_scale*link->left,m_scale*link->top), QPoint(m_scale*link->right,m_scale*link->bottom));
            if (rect.contains(QPoint(x,y)))
            {
                inLink = true;
                linkIAmIn = link;
                break;
            }
        }
    }

    if (m_inLink != inLink)
    {
        if (inLink)
        {
            QApplication::setOverrideCursor(Qt::PointingHandCursor);
//            qDebug("setting hand");
            m_link = linkIAmIn;
        }
        else
        {
            QApplication::restoreOverrideCursor();
//            qDebug("clearing hand");
            m_link = NULL;
        }

        m_inLink = inLink;
    }

}


