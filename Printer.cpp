#include "Printer.h"
#include "Window.h"
#include "Document.h"
#include "QtUtil.h"

#include <QtPrintSupport>
#include <QApplication>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QProgressDialog>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QTimer>
#include <QFileInfo>

#ifdef USE_CUPS
#include <cups/cups.h>
#endif

#include <ApplicationServices/ApplicationServices.h>

Printer::Printer(QObject *parent) : QObject(parent)
{
}

#ifdef USE_CUPS
QString jobStateName(ipp_jstate_t state)
{
    switch (state)
    {
        case IPP_JOB_PENDING:        return QString("IPP_JOB_PENDING");       break;
        case IPP_JOB_HELD:           return QString("IPP_JOB_HELD");          break;
        case IPP_JOB_PROCESSING:     return QString("IPP_JOB_PROCESSING");    break;
        case IPP_JOB_STOPPED:        return QString("IPP_JOB_STOPPED");       break;
        case IPP_JOB_CANCELED:       return QString("IPP_JOB_CANCELED");      break;
        case IPP_JOB_ABORTED:        return QString("IPP_JOB_ABORTED");       break;
        case IPP_JOB_COMPLETED:      return QString("IPP_JOB_COMPLETED");     break;
        default:  return QString("unknown state")+QString::number(state);     break;
    }
}

ipp_jstate_t getJobState(int jobID)
{
    int num_jobs;
    cups_job_t *jobs;
    ipp_jstate_t job_state = IPP_JOB_PENDING;

    //  get the jobs
    num_jobs = cupsGetJobs(&jobs, NULL, 1, -1);

    //  Loop to find my job
    job_state = IPP_JOB_COMPLETED;

    for (int i = 0; i < num_jobs; i ++)
    {
        if (jobs[i].id == jobID)
        {
            //  this is me
            job_state = jobs[i].state;
            break;
        }
    }

    //  Free the job array
    cupsFreeJobs(num_jobs, jobs);

//    qDebug("monitor: job %d is in state %s", jobID, jobStateName(job_state).toStdString().c_str());

    return job_state;
}

#endif

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
#ifdef USE_CUPS
        pdfPrint (&printer, m_window->getPath(), fromPage, toPage);
#else
        bitmapPrint (&printer, fromPage, toPage);
#endif
    }
    else if (fileInfo.suffix().toLower() == QString("xps"))
    {
#ifdef USE_CUPS
        //  put the result into the temp folder
        QString newPath = QtUtil::getTempFolderPath() + fileInfo.fileName() + ".pdf";

        //  create a process to do the conversion
        QProcess *process = new QProcess(this);

        //  construct the command
        QString command = "\"" + QtUtil::getGxpsPath() + "\"";
        command += " -dNOPAUSE -sDEVICE=pdfwrite ";
        command += "-sOutputFile=\"" + newPath + "\"";
        command += " ";
        command += "\"" + m_window->getPath() + "\"";
//        qDebug("command is: %s", command.toStdString().c_str());

        //  do it, and wait
        process->start(command);
        process->waitForFinished();

        //  print the new one
        pdfPrint (&printer, newPath, fromPage, toPage);
#else
        bitmapPrint (&printer, fromPage, toPage);
#endif
    }
    else
    {
        //  TODO: convert to PDF.  But for now,
        //  do it with bitmaps.
        bitmapPrint (&printer, fromPage, toPage);
    }
}

#ifdef USE_CUPS
void Printer::pdfPrint (QPrinter *printer, QString path, int fromPage, int toPage)
{
    //  set up options
    int num_options = 0;
    cups_option_t *options = NULL;

    //  construct an option for the page range.
    QString range = QString::number(fromPage) + "-" + QString::number(toPage);
    num_options = cupsAddOption("page-ranges", range.toStdString().c_str(), num_options, &options);

    //  start it
    m_jobID = cupsPrintFile (printer->printerName().toStdString().c_str(), path.toStdString().c_str(),
                             path.toStdString().c_str(),
                             num_options, options);

    cupsFreeOptions(num_options, options);

    ipp_jstate_t job_state = getJobState(m_jobID);

    if (job_state == IPP_JOB_COMPLETED ||
        job_state == IPP_JOB_PENDING   ||
        job_state == IPP_JOB_PROCESSING   )
    {

#ifdef _QT_MAC

        //  launch corresponding queue app

        //  find the queue name by running though the CUPS destinations
        //  looking for a match.  The queue name is stored in the
        //  "printer-info" option.

        QString queueName("");

        cups_dest_t *dests = NULL;
        int numDests = cupsGetDests(&dests);
        for (int i = 0; i < numDests; i++)
        {
            cups_dest_t dest = dests[i];
            if (printer->printerName().compare(QString(dest.name))==0)
            {
                for (int j = 0; j < dest.num_options; j++)
                {
                    cups_option_t opt = dest.options[j];
                    if (QString(opt.name).compare(QString("printer-info"))==0)
                    {
                        queueName = QString(opt.value);
                        break;
                    }
                }
            }
            if (!queueName.isEmpty())
                break;
        }
        cupsFreeDests(numDests, dests);

        //  if we found the queue, launch it
        if (!queueName.isEmpty())
        {
            //  app is in ~/Library/Printers/queueName.app
            QString s;
            s += "\"";
            s += QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
            s += "/Library/Printers/";
            s += queueName;
            s += ".app/Contents/MacOS/PrinterProxy";
            s += "\"";

            //  TODO: run in the background
            QProcess process;
            bool bRet = process.startDetached(s);
        }

#else
        QMessageBox::information(m_window, "", "Print job created.");
#endif
    }
    else
    {
        QMessageBox::information(m_window, "", "Error creating print job.");
    }
}
#endif

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

    //    int numPages = toPage-fromPage+1;
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

//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------

#if 0

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

    //  get scale factor based on printer's resolution
    double scalePrint = printer.resolution() / 72;

    //  those were 1-based, so subtract
    fromPage -= 1;
    toPage -= 1;

    //  begin printing
    QPainter *painter = new QPainter();
    painter->begin(&printer);

    //  for each page
    int page = fromPage;
    while (page <= toPage)
    {
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

    //  end printing
    painter->end();
    delete painter;
    painter = NULL;

    QMessageBox::information(m_window, "", "Printing is complete.");
}

#endif


