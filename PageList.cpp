#include <QPoint>
#include <QMouseEvent>
#include <QClipboard>
#include <QMessageBox>

#include "PageList.h"

PageList::PageList()
{
}

void PageList::onMousePress(QEvent *e)
{
    //  right mouse button?
    if (((QMouseEvent *)e)->button()==Qt::RightButton)
    {
        onRightClick(e);
        return;
    }

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

void PageList::setSearchText(int nPage, std::vector<SearchItem> *items)
{
    images()[nPage].setSearchText(items);
}

void PageList::clearSearchText()
{
    int nPages = document()->GetPageCount();
    for (int np=0; np<nPages; np++)
    {
        images()[np].clearSearchText();
    }
}

void PageList::hilightSearchText(SearchItem *item)
{
    //  tell each page to hilight the item.
    //  only one page will actually do it.
    int nPages = document()->GetPageCount();
    for (int np=0; np<nPages; np++)
    {
        images()[np].hilightSearchText(item);
    }

    //  scroll to put the item in view
    int pageNumber = item->pageNumber;
    QWidget *thePage = &(images()[pageNumber]);
    QPoint p(getScale()*item->left, getScale()*item->top);
    p = thePage->mapToParent(p);
    getScrollArea()->ensureVisible(p.x(), p.y(), 50, 200);
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
            TextBlock *block = (document()->blockList()[pageNumber].at(kk));
            QRect rect ( QPoint(scale*block->X,scale*block->Y),
                         QPoint(scale*(block->X+block->Width),scale*(block->Y+block->Height)));
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

bool isBetween (int val, int a, int b)
{
    if (a<=val && val<=b)
        return true;
    if (b<=val && val<=a)
        return true;
    return false;
}

int charIndex(TextLine *line, ImageWidget *widget, QPoint pos)
{
    int num_chars = line->char_list->size();
    for (int ii = 0; ii < num_chars; ii++)
    {
        TextCharacter *theChar = (line->char_list->at(ii));
        QRect cRect ( widget->scale()*theChar->X, widget->scale()*theChar->Y,
                      widget->scale()*theChar->Width, widget->scale()*theChar->Height);
        cRect.setTopLeft(widget->mapToGlobal(cRect.topLeft()));
        cRect.setBottomRight(widget->mapToGlobal(cRect.bottomRight()));

        if (ii==0)
        {
            if (pos.x()<cRect.left())
                return 0;
        }
        else
        {
            if (isBetween(pos.x(), cRect.left(), cRect.right()))
                return ii;
            TextCharacter *theChar2 = (line->char_list->at(ii-1));
            QRect cRect2 ( widget->scale()*theChar2->X, widget->scale()*theChar2->Y,
                           widget->scale()*theChar2->Width, widget->scale()*theChar2->Height);
            cRect2.setTopLeft(widget->mapToGlobal(cRect2.topLeft()));
            cRect2.setBottomRight(widget->mapToGlobal(cRect2.bottomRight()));

            if (isBetween(pos.x(), cRect2.right(), cRect.left()))
                return ii;
        }
    }

    //  not found, return last one
    return num_chars-1;
}

void PageList::updateSelection(QEvent *e)
{
    //  new mouse location in global coords
    QPoint newPosGlobal = getScrollArea()->mapToGlobal(((QMouseEvent *)e)->pos());

    bool movingDown  = (newPosGlobal.y() >= m_origin.y());
    bool movingRight = (newPosGlobal.x() >= m_origin.x());

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
            TextBlock *block = (document()->blockList()[pageNumber].at(kk));

            int num_lines = block->line_list->size();
            for (int jj = 0; jj < num_lines; jj++)
            {
                TextLine *line = (block->line_list->at(jj));

                //  global rect of the current line
                QRect lineRect ( widget->scale()*line->X, widget->scale()*line->Y,
                                 widget->scale()*line->Width, widget->scale()*line->Height);
                lineRect.setTopLeft(widget->mapToGlobal(lineRect.topLeft()));
                lineRect.setBottomRight(widget->mapToGlobal(lineRect.bottomRight()));

                //  check to see if the line should be included

                bool bAdd = false;
                if (lineRect.contains(m_origin) || lineRect.contains(newPosGlobal))
                    bAdd = true;
                if (isBetween(lineRect.top(), m_origin.y(), newPosGlobal.y()))
                    bAdd = true;
                if (isBetween(lineRect.bottom(), m_origin.y(), newPosGlobal.y()))
                    bAdd = true;

                if ( bAdd)
                {
                    int num_chars = line->char_list->size();

                    if (lineRect.contains(m_origin) && lineRect.contains(newPosGlobal))
                    {
                        int i1 = charIndex(line, widget, m_origin);
                        int i2 = charIndex(line, widget, newPosGlobal);
                        if (movingRight)
                        {
                            widget->addToSelection(line, i1, i2);
                        }
                        else
                        {
                            widget->addToSelection(line, i2, i1);
                        }
                    }
                    else if (lineRect.contains(m_origin))
                    {
                        int i1 = charIndex(line, widget, m_origin);
                        if (movingDown)
                        {
                            widget->addToSelection(line, i1, num_chars-1);
                        }
                        else
                        {
                            widget->addToSelection(line, 0, i1);
                        }
                    }
                    else if (lineRect.contains(newPosGlobal))
                    {
                        int i1 = charIndex(line, widget, newPosGlobal);
                        if (movingDown)
                        {
                            widget->addToSelection(line, 0, i1);
                        }
                        else
                        {
                            widget->addToSelection(line, i1, num_chars-1);
                        }
                    }
                    else
                    {
                        widget->addToSelection(line);
                    }
                }
                else
                {
                    widget->removeFromSelection(line);
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

void PageList::onRightClick(QEvent *e)
{
    QPoint origin = getScrollArea()->mapToGlobal(((QMouseEvent *)e)->pos());

    QMenu myMenu;
    QAction *copy        = myMenu.addAction("Copy");
    QAction *deselect    = myMenu.addAction("Deselect All");
    QAction *selectLine  = myMenu.addAction("Select Line");
    QAction *selectBlock = myMenu.addAction("Select Block");
    QAction *selectPage  = myMenu.addAction("Select Page");
    QAction *selectAll   = myMenu.addAction("Select All");

    QAction* selectedItem = myMenu.exec(origin);

    if (selectedItem==copy)
        onMenuCopy();
    else if (selectedItem==deselect)
        onMenuDeselect();
    else if (selectedItem==selectLine)
        onMenuSelectLine();
    else if (selectedItem==selectBlock)
        onMenuSelectBlock();
    else if (selectedItem==selectPage)
        onMenuSelectPage();
    else if (selectedItem==selectAll)
        onMenuSelectAll();
    else
        ;//nothing chosen

}

void PageList::onMenuCopy()
{
    copyText();
}

void PageList::onMenuDeselect()
{
    deselectText();
}

void PageList::onMenuSelectLine()
{

}

void PageList::onMenuSelectBlock()
{

}

void PageList::onMenuSelectPage()
{

}

void PageList::onMenuSelectAll()
{
    selectAllText();
}

