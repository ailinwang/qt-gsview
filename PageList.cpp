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
    QPoint posGlobal = getScrollArea()->mapToGlobal(((QMouseEvent *)e)->pos());

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

                //  rect of the line, in currently-scaled widget coords.
                double scale = widget->scale();
                QRect rect ( QPoint(scale*line.X,scale*line.Y),
                             QPoint(scale*(line.X+line.Width),scale*(line.Y+line.Height)));

                //  transform to globals
                QPoint p(rect.left(), rect.top());
                p = widget->mapToGlobal(p);
                rect.setLeft(p.x());
                rect.setTop(p.y());

                //  if the top or bottom of the line rect is
                //  between the starting and current point of selection, add it

                bool bAdd = false;
                if (m_origin.y() <= rect.top() && rect.top()<= posGlobal.y())
                    bAdd = true;
                if (posGlobal.y() <= rect.top() && rect.top()<= m_origin.y())
                    bAdd = true;

                //  these don't seem to work and I don't know why
//                if (m_origin.y() <= rect.bottom() && rect.bottom()<= posGlobal.y())
//                    bAdd = true;
//                if (posGlobal.y() <= rect.bottom() && rect.bottom()<= m_origin.y())
//                    bAdd = true;

                if ( bAdd)
                {
                    widget->addToSelection(&(block.line_list->at(jj)));
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
