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

        QPen lineStyle (QColor("#24A719"), 2);
        painter.setPen(lineStyle);
        painter.drawRect(rect);
    }

    if (showLinks() && !thumbnail())
    {
        int n = m_document->ComputeLinks(m_pageNumber);
        for (int i=0;i<n;i++)
        {
            Link *link = m_document->GetLink(m_pageNumber, i);
            if (link != NULL)
            {
                //  draw transparent blue filled rect
                QRect rect ( QPoint(m_scale*link->left,m_scale*link->top),
                             QPoint(m_scale*link->right,m_scale*link->bottom));
                painter.fillRect(rect, QBrush(QColor("#506EB3E8")));  //  transparent blue
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
        if (m_mouseInLink && !thumbnail())
        {
            //  in a link, so launch the url
            QUrl url(QString(m_mouseInLink->Uri.c_str()));
            QDesktopServices::openUrl(url);
        }
        else if (thumbnail())
        {
            //  not in a link, post this event to the window
            QApplication::postEvent(this->window(), new ImageClickedEvent(m_pageNumber));
        }
    }

    //  done
    return result;
}

void ImageWidget::mouseMoveEvent( QMouseEvent * event )
{
    if (!thumbnail())
    {
        //  get coordinates of the cursor relative to the widget
        //
        int x = event->x();
        int y = event->y();

        //  determine if the mouse is inside a link, and which one.
        int n = m_document->ComputeLinks(m_pageNumber);
        Link *linkIAmIn = NULL;
        for (int i=0; i<n; i++)
        {
            Link *link = m_document->GetLink(m_pageNumber, i);
            if (link != NULL)
            {
                //  make sure the rect is scaled
                QRect rect( QPoint(m_scale*link->left,m_scale*link->top),
                            QPoint(m_scale*link->right,m_scale*link->bottom));
                if (rect.contains(QPoint(x,y)))
                {
                    //  we're in a link
                    linkIAmIn = link;
                    break;
                }
            }
        }

        //  if the link we're in has changed,
        if (m_mouseInLink != linkIAmIn)
        {
            if (linkIAmIn)
            {
                //  in a link, so show the pointing hand
                this->setCursor(Qt::PointingHandCursor);
                qApp->processEvents();
            }
            else
            {
                //  not in a link, so restore normal cursor.
                this->setCursor(Qt::ArrowCursor);
                qApp->processEvents();
            }

            //  remember new value
            m_mouseInLink = linkIAmIn;
        }

    }

}


