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

    if (m_selected_lines.size()>0)
    {
        //  we have selected things.  Hilight them.

        for (int jj = 0; jj < m_selected_lines.size(); jj++)
        {
            TextLine line = *(m_selected_lines.at(jj));

            double scale = this->scale();
            QRect lrect ( QPoint(scale*line.X,scale*line.Y),
                          QPoint(scale*(line.X+line.Width),scale*(line.Y+line.Height)));
            painter.setPen(QPen(QColor("#ff0000"), 1));
            painter.drawRect(lrect);
        }

    }

//    //  TESTTESTTEST:  hilight blocks.  Just to see that we've
//    //  got the blocks right
//    if (!thumbnail())
//    {
//        HilightBlocks (&painter, m_scale, m_pageNumber, false, true, false);  // blocks, lines, chars
//    }
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
//    bool result = QLabel::eventFilter(obj, event);

    //  post event if it was a click
    if (event->type() == QEvent::MouseButtonRelease)
    {
        if (m_mouseInLink && !thumbnail())
        {
            //  handle URI and GOTO links
            if (m_mouseInLink->Type == LINK_URI)
            {
                QUrl url(QString(m_mouseInLink->Uri.c_str()));
                QDesktopServices::openUrl(url);
            }
            else if (m_mouseInLink->Type == LINK_GOTO)
            {
                ((Window *)this->window())->goToPage(m_mouseInLink->PageNum);
            }
        }
        else if (thumbnail())
        {
            //  not in a link, post this event to the window
            QApplication::postEvent(this->window(), new ImageClickedEvent(m_pageNumber));
        }
    }

    //  done
    return false;
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

        //  if the link we're in has changed, show/hide the hand cursor
        if (m_mouseInLink != linkIAmIn)
        {
//            if (linkIAmIn)
//            {
//                //  in a link, so show the pointing hand
//                this->setCursor(Qt::PointingHandCursor);
//                qApp->processEvents();
//            }
//            else
//            {
//                //  not in a link, so restore normal cursor.
//                this->setCursor(Qt::ArrowCursor);
//                qApp->processEvents();
//            }

            //  remember new value
            m_mouseInLink = linkIAmIn;

            //  if we're currently in a link, show/hide a tool tip
            if (m_mouseInLink)
            {
                if (linkIAmIn->Type==LINK_GOTO)
                {
                    QToolTip::showText(event->globalPos(), QString("go to page ")+QString::number(linkIAmIn->PageNum+1));
                }
                else if (linkIAmIn->Type==LINK_URI)
                {
                    QToolTip::showText(event->globalPos(), QString(linkIAmIn->Uri.c_str()));
                }
                else
                {
                    QToolTip::hideText();
                }
            }
            else
            {
                QToolTip::hideText();
            }
            qApp->processEvents();

        }
    }

    QLabel::mouseMoveEvent(event);
}

void ImageWidget::clearSelection()
{
    m_selected_lines.clear();
    update();
}

void ImageWidget::addToSelection(TextLine *line)
{
    bool found = false;
    for (unsigned int i=0; i<m_selected_lines.size(); i++)
    {
        TextLine *x = m_selected_lines.at(i);
        if (x->PageNumber==line->PageNumber &&
            x->BlockNumber==line->BlockNumber &&
            x->LineNumber==line->LineNumber)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        qDebug("addToSelection p=%d b=%d l=%d", line->PageNumber, line->BlockNumber, line->LineNumber );
        m_selected_lines.push_back(line);
    }
}

void ImageWidget::HilightBlocks (QPainter *painter, double scale, int pageNumber,
                                 bool drawBlocks, bool drawLines, bool drawChars)
{
    int num_blocks = m_document->blockList()[pageNumber].size();
    for (int kk = 0; kk < num_blocks; kk++)
    {
        TextBlock block = m_document->blockList()[pageNumber].at(kk);

        if (drawBlocks)
        {
            QRect brect ( QPoint(scale*block.X,scale*block.Y),
                          QPoint(scale*(block.X+block.Width),scale*(block.Y+block.Height)));
            painter->fillRect(brect, QBrush(QColor("#506EB3E8")));
        }

        int num_lines = block.line_list->size();
        for (int jj = 0; jj < num_lines; jj++)
        {
            TextLine line = block.line_list->at(jj);

            if (drawLines)
            {
                QRect lrect ( QPoint(scale*line.X,scale*line.Y),
                              QPoint(scale*(line.X+line.Width),scale*(line.Y+line.Height)));
                painter->setPen(QPen(QColor("#ff0000"), 1));
                painter->drawRect(lrect);
            }

            int num_chars = line.char_list->size();
            for (int ii = 0; ii < num_chars; ii++)
            {
                TextCharacter theChar = line.char_list->at(ii);

                if (drawChars)
                {
                    QRect crect ( QPoint(scale*theChar.X,scale*theChar.Y),
                                  QPoint(scale*(theChar.X+theChar.Width),scale*(theChar.Y+theChar.Height)));
                    painter->setPen(QPen(QColor("#0000ff"), 1));
                    painter->drawRect(crect);
                }
            }
        }
    }

}
