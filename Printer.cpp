#include "Printer.h"
#include "Window.h"
#include "Document.h"
#include "QtUtil.h"

#include <QtWidgets>
#include <QApplication>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QProgressDialog>
#include <QMessageBox>

Printer::Printer(Window *win)
{
    m_window = win;
}

void Printer::print()
{
    //  get the printer
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dialog = new QPrintDialog(&printer, m_window);
    dialog->setWindowTitle(QString("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return;
    dialog->hide();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::sendPostedEvents();

    //  get scale factor based on printer's resolution
    double scalePrint = printer.resolution() / 72;

    //  figure out printing range
    int fromPage = 1;
    int toPage = m_window->document()->GetPageCount();
    if (printer.fromPage()>0)
        fromPage = printer.fromPage();
    if (printer.toPage()>0)
        toPage = printer.toPage();
    if (toPage>m_window->document()->GetPageCount())
        toPage = m_window->document()->GetPageCount();

    //  those were 1-based, so subtract
    fromPage -= 1;
    toPage -= 1;

    //  begin printing
    QPainter *painter = new QPainter();
    painter->begin(&printer);

    int numPages = toPage-fromPage+1;
    QProgressDialog progress("Printing", "Cancel", 0, numPages, m_window);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::sendPostedEvents();

    bool cancelled = false;

    //  for each page
    int page = fromPage;
    while (page <= toPage)
    {
        progress.setValue(page-fromPage+1);
        QString message; message.sprintf("Printing %d of %d ...", page, numPages);
        progress.setLabelText(message);
        QApplication::sendPostedEvents();

        if (progress.wasCanceled())
        {
            cancelled = true;
            break;
        }

        //  if not the first page, start a new page
        if (page != fromPage)
            printer.newPage();

        //  compute page size
        point_t pageSize;
        m_window->document()->GetPageSize(page, scalePrint, &pageSize);

        //  render a bitmap
        int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;
        Byte *bitmap = new Byte[numBytes];
        m_window->document()->RenderPage(page, scalePrint, bitmap, pageSize.X, pageSize.Y, m_window->getShowAnnotations());

        //  copy to printer
        QImage *myImage = QtUtil::QImageFromData (bitmap, (int)pageSize.X, (int)pageSize.Y);
        painter->drawImage(0, 0, *myImage);

        delete myImage;
        delete bitmap;

        page++;
    }

    progress.hide();
    QApplication::sendPostedEvents();

    QApplication::restoreOverrideCursor();
    QApplication::sendPostedEvents();

    //  end printing
    if (cancelled)
    {
        QMessageBox::information(m_window, "", "Printing was cancelled.");
    }
    else
    {
        QMessageBox::information(m_window, "", "Printing is complete.");
    }
    painter->end();

    //  don't need the painter any more
    delete painter;
    painter = NULL;
}

