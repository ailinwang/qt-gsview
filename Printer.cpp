#include "Printer.h"
#include "Window.h"
#include "Document.h"
#include "QtUtil.h"

#include <QApplication>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QProgressDialog>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QTimer>
#include <QFileInfo>

#include <cups/cups.h>

Printer::Printer(QObject *parent) : QObject(parent)
{
}

void Printer::printFinish()
{
    //  release options
    cupsFreeOptions(num_options, options);

    QMessageBox::information(m_window, "", "Printing is complete.");
}

void Printer::monitor()
{
    int num_jobs;
    cups_job_t *jobs;
    ipp_jstate_t job_state = IPP_JOB_PENDING;

    num_jobs = cupsGetJobs(&jobs, NULL, 1, -1);

    //  Loop to find my job
    job_state = IPP_JOB_COMPLETED;

    for (int i = 0; i < num_jobs; i ++)
    {
        if (jobs[i].id == m_jobID)
        {
            //  this is me
            qDebug("I am found = %d", m_jobID);
            job_state = jobs[i].state;
            break;
        }
    }

    //  Free the job array
    cupsFreeJobs(num_jobs, jobs);

    if (job_state == IPP_JOB_COMPLETED)
    {
        qDebug("print finished = %d", m_jobID);
        printFinish();
        m_monitor->stop();
        delete m_monitor;
        m_monitor = NULL;
    }
    else
    {
        qDebug("still printing = %d", m_jobID);
    }
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

    //  figure out printing range
    int fromPage = 1;
    int toPage = m_window->document()->GetPageCount();
    if (printer.fromPage()>0)
        fromPage = printer.fromPage();
    if (printer.toPage()>0)
        toPage = printer.toPage();
    if (toPage>m_window->document()->GetPageCount())
        toPage = m_window->document()->GetPageCount();

    QFileInfo fileInfo (m_window->getPath());

    if (fileInfo.suffix().toLower() == QString("pdf"))
    {
        //  print it as is
        pdfPrint (&printer, m_window->getPath(), fromPage, toPage);
    }

    else
    {
        //  TODO: convert to PDF.  But for now,
        //  do it with bitmaps.
        bitmapPrint (&printer, fromPage, toPage);
    }
}

void Printer::pdfPrint (QPrinter *printer, QString path, int fromPage, int toPage)
{
    //  set up options
    num_options = 0;
    options = NULL;

    //  construct an option for the page range.
    QString range = QString::number(fromPage) + "-" + QString::number(toPage);
    num_options = cupsAddOption("page-ranges", range.toStdString().c_str(), num_options, &options);

    //  start it
    m_jobID = cupsPrintFile (printer->printerName().toStdString().c_str(), path.toStdString().c_str(),
                                     path.toStdString().c_str(),
                               num_options, options);

    //  start the monitor
    qDebug("job started = %d", m_jobID);
    m_monitor = new QTimer(this);
    connect (m_monitor, SIGNAL(timeout()), this, SLOT(monitor()));
    m_monitor->start(2000);
}

void Printer::bitmapPrint (QPrinter *printer, int fromPage, int toPage)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::sendPostedEvents();

    //  get scale factor based on printer's resolution
    double scalePrint = printer->resolution() / 72;

    //  those were 1-based, so subtract
    fromPage -= 1;
    toPage -= 1;

    //  begin printing
    QPainter *painter = new QPainter();
    painter->begin(printer);

    int numPages = toPage-fromPage+1;
//    QProgressDialog progress("Printing", "Cancel", 0, numPages, m_window);
//    progress.setWindowModality(Qt::WindowModal);
//    progress.show();
//    QApplication::sendPostedEvents();

    bool cancelled = false;

    //  for timing
//    QElapsedTimer timer;
//    qint64 nanoSec;
//    timer.start();

    //  for each page
    int page = fromPage;
    while (page <= toPage)
    {
//        progress.setValue(page-fromPage+1);
//        QString message; message.sprintf("Printing %d of %d ...", page, numPages);
//        progress.setLabelText(message);
//        QApplication::sendPostedEvents();

//        if (progress.wasCanceled())
//        {
//            cancelled = true;
//            break;
//        }

        //  if not the first page, start a new page
        if (page != fromPage)
            printer->newPage();

        //  compute page size
        point_t pageSize;
        m_window->document()->GetPageSize(page, scalePrint, &pageSize);

//        timer.restart();  //  restart timer

        //  render a bitmap
        int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;
        Byte *bitmap = new Byte[numBytes];
        m_window->document()->RenderPage(page, scalePrint, bitmap, pageSize.X, pageSize.Y, m_window->getShowAnnotations());

        //  copy to printer
        QImage *myImage = QtUtil::QImageFromData (bitmap, (int)pageSize.X, (int)pageSize.Y);
        painter->drawImage(0, 0, *myImage);

        delete myImage;
        delete bitmap;

//        nanoSec = timer.nsecsElapsed();  //  get elapsed
//        double elapsed = double(nanoSec)/1000000000;  //  seconds
//        qDebug("printed page %d in %f", page, elapsed);

        page++;
    }

//    progress.hide();
//    QApplication::sendPostedEvents();

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
