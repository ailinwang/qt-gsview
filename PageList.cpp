#include <QPoint>
#include <QMouseEvent>
#include <QClipboard>

#include "PageList.h"

PageList::PageList()
{
}

void PageList::onMousePress(QEvent *e)
{
    //  if the shift key is pressed,
    //  update the current selection instead of starting a new one.
    QMouseEvent *me = ((QMouseEvent *)e);
    if(me->modifiers() & Qt::ShiftModifier)
    {
        updateSelection(e);
        return;
    }

    //  clear current selections
    deselectText();

    //  remember mouse location in global coordinates
    m_origin = getScrollArea()->mapToGlobal(((QMouseEvent *)e)->pos());
}

void PageList::deselectText()
{
    int nPages = document()->GetPageCount();
    for (int i=0; i<nPages; i++)
        images()[i].clearSelection();
}

void PageList::selectAllText()
{
    int nPages = document()->GetPageCount();
    for (int i=0; i<nPages; i++)
    {
        document()->ComputeTextBlocks(i);
        images()[i].selectAllText();
    }
}

void PageList::copyText()
{
    QString allText;
    int nPages = document()->GetPageCount();
    for (int i=0; i<nPages; i++)
    {
        QString pageText = images()[i].selectedText();
        allText += pageText;
    }

    //  now put it on the clipboard
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(allText);
}

void PageList::onMouseRelease(QEvent *e)
{
    UNUSED(e);
}

void PageList::manageCursor(QEvent *e)
{
    //  current mouse location
    QPoint pos = ((QMouseEvent *)e)->pos();
    QPoint posGlobal = getScrollArea()->mapToGlobal(pos);

    //  are we "in" a page?
    ImageWidget *widget = dynamic_cast<ImageWidget*>(qApp->widgetAt(QCursor::pos()));
    if (widget != NULL)
    {
        //  compute (or retrieve) text blocks for this page
        int pageNumber = widget->pageNumber();
        document()->ComputeTextBlocks(pageNumber);

        //  see if we're inside one
        QPoint posPage = widget->mapFromGlobal(posGlobal);
        int num_blocks = document()->blockList()[pageNumber].size();
        bool bInside = false;
        double scale = widget->scale();
        for (int kk = 0; kk < num_blocks; kk++)
        {
            TextBlock block = document()->blockList()[pageNumber].at(kk);
            QRect rect ( QPoint(scale*block.X,scale*block.Y),
                         QPoint(scale*(block.X+block.Width),scale*(block.Y+block.Height)));
            if ( rect.contains(posPage))
            {
                bInside = true;
                break;
            }
        }

        if (bInside)
        {
            getScrollArea()->setCursor(Qt::IBeamCursor);
            qApp->processEvents();
        }
        else
        {
            getScrollArea()->setCursor(Qt::ArrowCursor);
            qApp->processEvents();
        }
    }

}

void PageList::updateSelection(QEvent *e)
{
    //  new mouse location in global coords
    QPoint newPosGlobal = getScrollArea()->mapToGlobal(((QMouseEvent *)e)->pos());

    //  which widget are we "in"?
    ImageWidget *widget = dynamic_cast<ImageWidget*>(qApp->widgetAt(QCursor::pos()));
    if (widget != NULL)
    {
        //  get blocks/lines/chars for this page (widget)
        int pageNumber = widget->pageNumber();
        document()->ComputeTextBlocks(pageNumber);
        int num_blocks = document()->blockList()[pageNumber].size();

        for (int kk = 0; kk < num_blocks; kk++)
        {
            TextBlock block = document()->blockList()[pageNumber].at(kk);

            int num_lines = block.line_list->size();
            for (int jj = 0; jj < num_lines; jj++)
            {
                TextLine line = block.line_list->at(jj);

                //  global rect of the current line
                QRect lineRect ( widget->scale()*line.X, widget->scale()*line.Y,
                                 widget->scale()*line.Width, widget->scale()*line.Height);
                lineRect.setTopLeft(widget->mapToGlobal(lineRect.topLeft()));
                lineRect.setBottomRight(widget->mapToGlobal(lineRect.bottomRight()));

                //  check to see if the line should be included

                bool bAdd = false;
                if (lineRect.contains(m_origin) || lineRect.contains(newPosGlobal))
                    bAdd = true;
                if (m_origin.y() <= lineRect.top() && lineRect.top()<= newPosGlobal.y())
                    bAdd = true;
                if (newPosGlobal.y() <= lineRect.top() && lineRect.top()<= m_origin.y())
                    bAdd = true;  //  upside down
                if (m_origin.y() <= lineRect.bottom() && lineRect.bottom()<= newPosGlobal.y())
                    bAdd = true;
                if (newPosGlobal.y() <= lineRect.bottom() && lineRect.bottom()<= m_origin.y())
                    bAdd = true;  //  upside down

                if ( bAdd)
                {
//                    bool upsideDown = (newPosGlobal.y() < m_origin.y());
//                    if (lineRect.contains(m_origin) && lineRect.contains(newPosGlobal))
//                    {
//                        //  line contains both points
//                        widget->addToSelection(&(block.line_list->at(jj)));
//                    }
//                    else if (lineRect.contains(m_origin))
//                    {
//                        //  line contains beginning
//                        widget->addToSelection(&(block.line_list->at(jj)));
//                    }
//                    else if (lineRect.contains(newPosGlobal))
//                    {
//                        //  line contains end
//                        widget->addToSelection(&(block.line_list->at(jj)));
//                    }
//                    else
//                    {
                        widget->addToSelection(&(block.line_list->at(jj)));
//                    }
                }
                else
                {
                    widget->removeFromSelection(&(block.line_list->at(jj)));
                }
            }
        }
        widget->update();
    }
}

void PageList::onMouseMove(QEvent *e)
{
    manageCursor(e);

    if(((QMouseEvent *)e)->buttons() == Qt::LeftButton)
    {
        updateSelection(e);
    }
}

bool PageList::onEvent(QEvent *e)
{
    if (e->type() == QMouseEvent::MouseButtonPress)
    {
        onMousePress(e);
    }
    else if (e->type() == QMouseEvent::MouseButtonRelease)
    {
        onMouseRelease(e);
    }

    else if (e->type() == QMouseEvent::MouseMove)
    {
        onMouseMove(e);
    }

    return false;
}
