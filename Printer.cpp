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
#include <QFileInfo>

#ifdef USE_CUPS
#include <cups/cups.h>
#endif

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

    return job_state;
}

#endif

void Printer::print()
{
    //  get the printer
    m_printer = new QPrinter(QPrinter::HighResolution);
    QPrintDialog *dialog = new QPrintDialog(m_printer, m_window);
    dialog->setWindowTitle(QString("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return;
    dialog->hide();

    //  figure out printing range
    int fromPage = 1;
    int toPage = m_window->document()->GetPageCount();
    if (m_printer->fromPage()>0)
        fromPage = m_printer->fromPage();
    if (m_printer->toPage()>0)
        toPage = m_printer->toPage();
    if (toPage>m_window->document()->GetPageCount())
        toPage = m_window->document()->GetPageCount();

    QFileInfo fileInfo (m_window->getPath());

    if (fileInfo.suffix().toLower() == QString("pdf"))
    {
        //  print it as is
#ifdef USE_CUPS
        pdfPrint (m_printer, m_window->getPath(), fromPage, toPage);
#else
        bitmapPrint (m_printer, fromPage, toPage);
#endif
    }
    else if (fileInfo.suffix().toLower() == QString("xps"))
    {
#ifdef USE_CUPS
        //  put the result into the temp folder
        QString newPath = QtUtil::getTempFolderPath() + fileInfo.fileName() + ".pdf";

        //  construct the command
        QString command = "\"" + QtUtil::getGxpsPath() + "\"";
        command += " -dNOPAUSE -sDEVICE=pdfwrite ";
        command += "-sOutputFile=\"" + newPath + "\"";
        command += " ";
        command += "\"" + m_window->getPath() + "\"";
//        qDebug("command is: %s", command.toStdString().c_str());

        //  create a process to do it, and wait
        QProcess *process = new QProcess(this);
        process->start(command);
        process->waitForFinished();

        //  print the new one
        pdfPrint (m_printer, newPath, fromPage, toPage);
#else
        bitmapPrint (m_printer, fromPage, toPage);
#endif
    }
    else
    {
        //  TODO: convert to PDF.  But for now,
        //  do it with bitmaps.
        bitmapPrint (m_printer, fromPage, toPage);
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
            process.startDetached(s);
        }

#else
        //  non-mac.  Do nothing.
#endif
        QMessageBox::information(m_window, "", "Print job created.");
    }
    else
    {
        QMessageBox::information(m_window, "", "Error creating print job.");
    }
}
#endif

void Printer::bitmapPrint (QPrinter *printer, int fromPage, int toPage)
{
    //  make a thread for printing, and a worker that runs ni the thread.
    m_printThread = new QThread;
    m_printWorker = new PrintWorker(m_window, printer, fromPage-1, toPage-1);  //  values given are  1-based
    m_printWorker->moveToThread(m_printThread);

    //  connect worker and thread signals
    connect(m_printThread, SIGNAL(started()),  m_printWorker, SLOT(process()));
    connect(m_printWorker, SIGNAL(finished()), m_printThread, SLOT(quit()));
    connect(m_printWorker, SIGNAL(finished()), m_printWorker, SLOT(deleteLater()));
    connect(m_printThread, SIGNAL(finished()), m_printThread, SLOT(deleteLater()));

    //  connect worker signals to slots in this class
    connect(m_printWorker, SIGNAL(finished()),       this, SLOT(printFinished()));
    connect(m_printWorker, SIGNAL(pagePrinted(int)), this, SLOT(pagePrinted(int)));

    //  put up a progress dialog
    m_pagesToPrint = toPage-fromPage+1;
    m_progress = new QProgressDialog ("Printing", "Cancel", 0, m_pagesToPrint, m_window);
    connect (m_progress, SIGNAL(canceled()), this, SLOT(onCanceled()));
    m_progress->setWindowModality(Qt::WindowModal);
    setProgress(0);
    m_progress->show();
    qApp->processEvents();

    //  start the thread
//    qDebug("starting thread");
    m_printThread->start();

    m_killed = false;
}

void Printer::pagePrinted(int nPage)
{
//    qDebug("received pagePrinted(%d)", nPage);

    //  update progress dialog
    setProgress(nPage);
}

void Printer::printFinished()
{
//    qDebug("received printFinished()");

    m_progress->hide();
    delete m_progress;

    if (m_killed)
    {
        QMessageBox::information(m_window, "", "Print job canceled.");
    }
    else
    {
        QMessageBox::information(m_window, "", "Print job created.");
    }
}

void Printer::onCanceled()
{
//    qDebug("received onCanceled()");

    m_printWorker->kill();  //  we should also receive printFinished
    m_killed = true;
}


void Printer::setProgress (int val)
{
//    qDebug("received setProgress()");

    m_progress->setValue(val);
    QString s = QString("Printed ")
                    + QString::number(val) + QString(" of ")
                    + QString::number(m_pagesToPrint) + QString(" pages...");
    m_progress->setLabelText(s);
    qApp->processEvents();
}


//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

PrintWorker::PrintWorker(Window *window, QPrinter *printer, int fromPage, int toPage)
{
    m_printer = printer;
    m_fromPage = fromPage;
    m_toPage = toPage;
    m_window = window;

//    qDebug("worker created");
}

PrintWorker::~PrintWorker()
{
//    qDebug("worker destroyed");
}

void PrintWorker::process()
{
//    qDebug("worker starting");

    //  get scale factor based on printer's resolution
    double scalePrint = m_printer->resolution() / 72;

    //  begin printing
    QPainter painter;
    painter.begin(m_printer);

    //  for each page
    int page = m_fromPage;
    int nPagesPrinted = 0;
    while (page <= m_toPage)
    {
        if (m_killed)
            break;

        //  if not the first page, start a new page
        if (page != m_fromPage)
            m_printer->newPage();

        //  compute page size
        point_t pageSize;
        m_window->document()->GetPageSize (page, scalePrint, &pageSize);

        //  render a bitmap
        int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;
        Byte *bitmap = new Byte[numBytes];
        m_window->document()->RenderPage(page, scalePrint, bitmap, pageSize.X, pageSize.Y, m_window->getShowAnnotations());

        if (m_killed)
            break;

        //  copy to printer
        QImage *myImage = new QImage(bitmap, (int)pageSize.X, (int)pageSize.Y, QImage::Format_ARGB32);
        painter.drawImage(0, 0, *myImage);

        delete myImage;
        delete bitmap;

        page++;

        if (m_killed)
            break;

        nPagesPrinted++;
//        qDebug("worker emitting pagePrinted(%d)", nPagesPrinted);
        emit pagePrinted(nPagesPrinted);
    }

    painter.end();

    if (m_killed)
        m_printer->abort();

    //  done!
//    qDebug("worker emitting finished()");
    emit finished();
}

