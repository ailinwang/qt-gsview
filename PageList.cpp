#include <QPoint>
#include <QMouseEvent>
#include <QClipboard>
#include <QMessageBox>
#include <QProcess>
#include <FileSave.h>

#include "PageList.h"
#include "QtUtil.h"
#include "SelectionFrame.h"
#include "Window.h"

PageList::PageList(Window *parent)
{
    m_window = parent;
}

void PageList::onMousePress(QEvent *e)
{
    QMouseEvent *me = ((QMouseEvent *)e);
    ImageWidget *widget = dynamic_cast<ImageWidget*>(qApp->widgetAt(QCursor::pos()));

    //  if we have an area selected, and the user does right-click, ignore.
    if (m_rubberBand && (me->button()==Qt::RightButton))
    {
        onRightClickArea(e);
        return;
    }

    m_selectingArea = false;
    if (NULL!=widget)
        widget->setSelectingArea(false);
    if (m_rubberBand)
    {
        m_rubberBand->hide();
        delete m_rubberBand;
        m_rubberBand=NULL;
    }

    //  selecting an area?
    if(me->modifiers() & Qt::ControlModifier)
    {
        deselectText();
        m_selectingArea = true;
        if (NULL!=widget)
            widget->setSelectingArea(true);
        m_rubberBandOrigin = widget->mapFromGlobal(QCursor::pos());
        m_rubberBand = new SelectionFrame(widget);
        m_rubberBand->setGeometry(QRect(m_rubberBandOrigin, QSize()));
        m_rubberBand->show();

        qApp->setOverrideCursor(Qt::CrossCursor);
        qApp->processEvents();

        //  which page?
        m_rubberbandpage = widget->pageNumber();

        return;
    }

    //  right mouse button?
    if (me->button()==Qt::RightButton)
    {
        onRightClick(e);
        return;
    }

    //  if the shift key is pressed,
    //  update the current selection instead of starting a new one.
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

bool PageList::isAreaSelected()
{
    if (m_rubberBand!=NULL && m_rubberBand->rect().width()>0 && m_rubberBand->rect().height()>0)
    {
        return true;
    }
    return false;
}

void PageList::saveSelection(FileSave *fileSave)
{
    //  get selection specifics
    QRect rect = m_rubberBand->geometry();
    ImageWidget *image = &(images()[m_rubberbandpage]);
    int imageHeight = image->height();

    int transX = rect.left();
    int transY = imageHeight-rect.top()-rect.height();
    int w = rect.width();
    int h = rect.height();

    //  de-scale these things
    double scale = getScale();
    w /= scale;
    h /= scale;
    transX /= scale;
    transY /= scale;

    fileSave->extractSelection (transX, transY, w, h, m_rubberbandpage, 300);
}

void PageList::zoom (double scale)
{
    ScrollingImageList::zoom (scale);

    //  adjust the size of the selection rectangle
    if (m_rubberBand!=NULL)
    {
        QRect r = m_rubberbandRect;

        QRect r2 (
                    r.left()  *scale/m_rubberbandScale,
                    r.top()   *scale/m_rubberbandScale,
                    r.width() *scale/m_rubberbandScale,
                    r.height()*scale/m_rubberbandScale  );

        m_rubberBand->setGeometry(r2);
    }

}

QString PageList::collectSelectedText()
{
    QString selectedText;
    int nPages = document()->GetPageCount();
    for (int i=0; i<nPages; i++)
    {
        QString pageText = images()[i].selectedText();
        selectedText += pageText;
    }
    return selectedText;
}

void PageList::copyText()
{
    QString selectedText = collectSelectedText();

    //  now put it on the clipboard
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(selectedText);
}

void PageList::onMouseRelease(QEvent *e)
{
    UNUSED(e);

    ImageWidget *widget = dynamic_cast<ImageWidget*>(qApp->widgetAt(QCursor::pos()));

    if (m_rubberBand && m_selectingArea)
    {
        m_rubberbandScale = getScale();
        m_rubberbandRect = m_rubberBand->geometry();
        m_selectingArea = false;
        if (NULL!=widget)
            widget->setSelectingArea(false);
        QApplication::restoreOverrideCursor();  //  default arrow cursor
        qApp->processEvents();

        return;
    }

    if (widget != NULL)
    {
        widget->onMouseRelease(e);
    }

}

void PageList::manageCursor(QEvent *e)
{
    if (m_controlKeyIsDown)
        return;

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
            qApp->setOverrideCursor(Qt::IBeamCursor);
            qApp->processEvents();
        }
        else
        {
            QApplication::restoreOverrideCursor();  //  default arrow cursor
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
    UNUSED(e);

    if (m_rubberBand && m_selectingArea)
    {
        ImageWidget *image = &(images()[m_rubberbandpage]);
        QPoint p = image->mapFromGlobal(QCursor::pos());

        m_rubberBand->setGeometry(QRect(m_rubberBandOrigin, p).normalized());

        return;
    }

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
    else if (e->type() == QKeyEvent::KeyPress)
    {
        QKeyEvent *ke = (QKeyEvent *)e;
        if (ke->key() == Qt::Key_Control)
        {
            m_controlKeyIsDown = true;
            qApp->setOverrideCursor(Qt::CrossCursor);
            qApp->processEvents();
        }
    }
    else if (e->type() == QKeyEvent::KeyRelease)
    {
        QKeyEvent *ke = (QKeyEvent *)e;
        if (ke->key() == Qt::Key_Control)
        {
            m_controlKeyIsDown = false;
            QApplication::restoreOverrideCursor();  //  default arrow cursor
            qApp->processEvents();
        }
    }

    return false;
}

void PageList::onRightClickArea(QEvent *e)
{
    //  point at which to show the menu
    QPoint origin = getScrollArea()->mapToGlobal(((QMouseEvent *)e)->pos());

    //  show a menu
    QMenu myMenu;
    QAction *select = myMenu.addAction(tr("Save Selection..."));
    QAction* selectedItem = myMenu.exec(origin);

    //  handle the result
    if (selectedItem==select)
    {
        m_window->saveSelection();
    }
    else
    {}  //nothing chosen

}


void PageList::onRightClick(QEvent *e)
{
    //  point at which to show the menu
    QPoint origin = getScrollArea()->mapToGlobal(((QMouseEvent *)e)->pos());

    //  currently-selected text
    QString selectedText = collectSelectedText();

    //  new menu
    QMenu myMenu;

    //  these two actions aren't used if nothing is selected
    QAction *copy=NULL;
    QAction *deselect=NULL;
    if (!selectedText.isEmpty())
    {
        copy        = myMenu.addAction(tr("Copy"));
        deselect    = myMenu.addAction(tr("Deselect All"));
    }

    //  more actions
    QAction *selectLine  = myMenu.addAction(tr("Select Line"));
    QAction *selectBlock = myMenu.addAction(tr("Select Block"));
    QAction *selectPage  = myMenu.addAction(tr("Select Page"));
    QAction *selectAll   = myMenu.addAction(tr("Select All"));

    //  show the menu
    QAction* selectedItem = myMenu.exec(origin);

    //  handle the result
    if (selectedItem==copy && !selectedText.isEmpty())
        onMenuCopy();
    else if (selectedItem==deselect && !selectedText.isEmpty())
        onMenuDeselect();
    else if (selectedItem==selectLine)
        onMenuSelectLine(e);
    else if (selectedItem==selectBlock)
        onMenuSelectBlock(e);
    else if (selectedItem==selectPage)
        onMenuSelectPage();
    else if (selectedItem==selectAll)
        onMenuSelectAll();
    else
        {}  //nothing chosen
}

void PageList::onMenuCopy()
{
    copyText();
}

void PageList::onMenuDeselect()
{
    deselectText();
}

void PageList::onMenuSelectLine(QEvent *e)
{
    //  first deselect everything
    deselectText();

    //  current mouse location
    QPoint pos = ((QMouseEvent *)e)->pos();
    QPoint posGlobal = getScrollArea()->mapToGlobal(pos);

    ImageWidget *widget = dynamic_cast<ImageWidget*>(qApp->widgetAt(QCursor::pos()));
    if (widget != NULL)
    {
        QPoint posPage = widget->mapFromGlobal(posGlobal);

        //  compute (or retrieve) text blocks for this page
        int pageNumber = widget->pageNumber();
        document()->ComputeTextBlocks(pageNumber);

        int num_blocks = document()->blockList()[pageNumber].size();
        double scale = widget->scale();
        for (int kk = 0; kk < num_blocks; kk++)
        {
            TextBlock *block = (document()->blockList()[pageNumber].at(kk));

            int num_lines = block->line_list->size();
            for (int jj = 0; jj < num_lines; jj++)
            {
                TextLine *line = (block->line_list->at(jj));

                //  see if we're inside this line
                QRect rect ( QPoint(scale*line->X,scale*line->Y),
                             QPoint(scale*(line->X+line->Width),scale*(line->Y+line->Height)));
                if (rect.contains(posPage))
                {
                    widget->addToSelection(line);
                    //  we're only selecting one line, so just return
                    widget->update();
                    return;
                }
            }
        }
    }
}

void PageList::onMenuSelectBlock(QEvent *e)
{
    //  first deselect everything
    deselectText();

    //  current mouse location
    QPoint pos = ((QMouseEvent *)e)->pos();
    QPoint posGlobal = getScrollArea()->mapToGlobal(pos);

    ImageWidget *widget = dynamic_cast<ImageWidget*>(qApp->widgetAt(QCursor::pos()));
    if (widget != NULL)
    {
        //  compute (or retrieve) text blocks for this page
        int pageNumber = widget->pageNumber();
        document()->ComputeTextBlocks(pageNumber);

        //  see if we're inside one
        QPoint posPage = widget->mapFromGlobal(posGlobal);
        int num_blocks = document()->blockList()[pageNumber].size();
        double scale = widget->scale();
        for (int kk = 0; kk < num_blocks; kk++)
        {
            TextBlock *block = (document()->blockList()[pageNumber].at(kk));
            QRect rect ( QPoint(scale*block->X,scale*block->Y),
                         QPoint(scale*(block->X+block->Width),scale*(block->Y+block->Height)));
            if ( rect.contains(posPage))
            {
                //  select all of the lines in this block
                int num_lines = block->line_list->size();
                for (int jj = 0; jj < num_lines; jj++)
                {
                    TextLine *line = (block->line_list->at(jj));
                    widget->addToSelection(line);
                }
                break;
            }
        }
        widget->update();
    }
}

void PageList::onMenuSelectPage()
{
    //  first deselect everything
    deselectText();

    ImageWidget *widget = dynamic_cast<ImageWidget*>(qApp->widgetAt(QCursor::pos()));
    if (widget != NULL)
    {
        //  compute (or retrieve) text blocks for this page
        int pageNumber = widget->pageNumber();
        document()->ComputeTextBlocks(pageNumber);

        int num_blocks = document()->blockList()[pageNumber].size();
        for (int kk = 0; kk < num_blocks; kk++)
        {
            TextBlock *block = (document()->blockList()[pageNumber].at(kk));
            {
                //  select all of the lines in this block
                int num_lines = block->line_list->size();
                for (int jj = 0; jj < num_lines; jj++)
                {
                    TextLine *line = (block->line_list->at(jj));
                    widget->addToSelection(line);
                }
            }
        }
        widget->update();
    }
}

void PageList::onMenuSelectAll()
{
    selectAllText();
}

