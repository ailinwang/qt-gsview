#include "FileSave.h"

#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QBasicTimer>

#include "QtUtil.h"

FileType fileTypes[] = {
    {"PDF",             "pdf" , "" },
    {"Linearizded PDF", "pdf" , "" },
    {"PDF 1.3",         "pdf" , "" },
    {"PDF/A-1 RGB",     "pdf" , "" },
    {"PDF/A-1 CMYK",    "pdf" , "" },
    {"PDF/A-2 RGB",     "pdf" , "" },
    {"PDF/A-2 CMYK",    "pdf" , "" },
    {"PDF/X-3 Gray",    "pdf" , "" },
    {"PDF/X-3 CMYK",    "pdf" , "" },
    {"PCL-XL",          "bin" , "" },
    {"XPS",             "xps" , "" },
    {"Text",            "txt" , "" },
    {"HTML",            "html", "" },
    {"XML",             "xml" , "" }
};
int numTypes = 14;

void FileSave::run()
{
    //  where is the desktop?
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QString desktop = desktopLocations.first();

    //  what are the file types?
    QString types;
    int i;
    for (i=0;i<numTypes;i++)
    {
        QString theFilter = fileTypes[i].filterName + QString(" (*.") + fileTypes[i].filterType + QString(")");
        fileTypes[i].filter = theFilter;
        if (i>0)
            types += ";;";
        types += theFilter;
    }

    //  set up the dialog
    QFileDialog dialog(m_window, "Save", desktop);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    //  INFO: It's hard to debug in the Qt IDE when we use the native file dialog.
    dialog.setOption(QFileDialog::DontUseNativeDialog, false);
    dialog.setNameFilter(types);

    //  get the name
    dialog.show();
    int result = dialog.exec();
    if (result == QDialog::Accepted)
    {
        QString fileName = dialog.selectedFiles().first();
        QString filter = dialog.selectedNameFilter();
        QString filterExt = QtUtil::extensionFromFilter(filter);
        QString fileExt = QtUtil::extensionFromPath(fileName);

        //  if the user gave no extension, use the filter
        if (fileExt.length()==0)
        {
            fileExt = filterExt;
            fileName += ".";
            fileName += filterExt;
        }

        //  if the extensions don't match, that's an error
        if (fileExt != filterExt)
        {
            QMessageBox::information (m_window, "", "you supplied a filename extension that does not match the format.");
            return;
        }

        //  find the matching type from the filter
        int index = -1;
        for (i=0;i<numTypes;i++)
        {
            if (filter==fileTypes[i].filter)
            {
                index = i;
                break;
            }
        }

        //  now do the save

        //  in all cases, the file dialog has asked the user about overwriting,
        //  so it's OK not to ask here.

        QString original = m_window->getPath();
        QString password;

        if (index==0)
        {
            //  regular PDF - just copy the file
            if (QFile::exists(fileName))
                QFile::remove(fileName);
            QFile::copy(original, fileName);
        }
        else if (index==1)
        {
            if (QFile::exists(fileName))
                QFile::remove(fileName);
            //  linearized PDF
            m_window->document()->PDFExtract (original.toStdString().c_str(), fileName.toStdString().c_str(),
                                              password.toStdString().c_str(),
                                            password.length()>0, true, -1, NULL);
        }
        else if (index==2)
        {
            //  use gs with options = "-dCompatibilityLevel=1.3";

            //  TODO: use an intermediate temp file.

            //  construct the command
            QString command = "\"" + QtUtil::getGsPath() + "\"";
            command += " -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -dCompatibilityLevel=1.3";
//            command += " -r72";  //  take this out later
            command += " -o \"" + fileName + "\"";
            command += " -f \"" + original + "\"";
            qDebug("command is: %s", command.toStdString().c_str());

            if (saveWithProgress(command))
            {
                //  TODO: swap temp file for real thing
            }
        }
        else
        {
            QApplication::restoreOverrideCursor();
            qApp->processEvents();

            //  NYI
            QString message = "saving is not yet implemented.<br/><br/>";
            message += "saving " + fileName + "<br/>as " + QString::number(index+1) + ". " + filter;
            QMessageBox::information (m_window, "", message);
        }
    }

}

bool FileSave::saveWithProgress(QString command)
{
    bool canceled = false;

    //  show a progress widget
    m_progressDialog = new QProgressDialog(m_window);
    connect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
    m_progressDialog->setMaximum(m_window->document()->GetPageCount());
    m_progressDialog->setValue(0);
    m_progressDialog->show();
    qApp->processEvents();

    m_timer = new QBasicTimer();
    m_timer->start(100,this);

    //  create a process to do the conversion.  During this time onReadyReadStandardOutput gets called;
    //  that's our opportunity to update the progress widget
    m_process = new QProcess(m_window);
    connect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
    m_process->start(command);
    m_process->waitForFinished();

    m_timer->stop();
    delete m_timer;

    if (m_progressDialog->wasCanceled())
    {
        QMessageBox::information (m_window, "", "Canceled.");
        canceled = true;
    }

    //  take down progress widget
    m_progressDialog->hide();
    qApp->processEvents();
    disconnect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
    delete m_progressDialog;

    //  disconnect/delete the process
    disconnect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
    delete m_process;
    m_process=NULL;

    return !canceled;
}

void FileSave::onReadyReadStandardOutput()
{
    char data[10000];
    while (m_process->canReadLine())
    {
        int nc = m_process->readLine(data, 10000);
        data[nc] = 0;

        qDebug("data: %s", data);

        //  every time we see "Page", crank the progress
        QString s(data);
        if (s.left(4).compare(QString("Page"))==0)
        {
            int val = m_progressDialog->value();
            m_progressDialog->setValue(val+1);
            qApp->processEvents();
        }
    }
}

void FileSave::onCanceled()
{
    qDebug("onCanceled");
    m_process->terminate();
}

void FileSave::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer->timerId())
    {
        if (m_progressDialog->wasCanceled())
        {
            qDebug("timerEvent onCanceled");
            m_process->terminate();
        }
    }
    else
    {
        QObject::timerEvent(event);
    }
}


