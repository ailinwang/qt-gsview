#include "FileSave.h"

#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QTemporaryFile>
#include <QtPrintSupport>

#include "QtUtil.h"
#include "MessagesDialog.h"


FileType fileTypes[] = {
    {"PDF",             "pdf" , "" },
    {"Linearizded PDF", "pdf" , "" },
    {"PDF 1.3",         "pdf" , "" },
//    {"PDF/A-1 RGB",     "pdf" , "" },
//    {"PDF/A-1 CMYK",    "pdf" , "" },
//    {"PDF/A-2 RGB",     "pdf" , "" },
//    {"PDF/A-2 CMYK",    "pdf" , "" },
//    {"PDF/X-3 Gray",    "pdf" , "" },
//    {"PDF/X-3 CMYK",    "pdf" , "" },
    {"PCL-XL",          "bin" , "" },
    {"XPS",             "xps" , "" },
    {"Text",            "txt" , "" },
    {"HTML",            "html", "" },
    {"XML",             "xml" , "" },
    {"PostScript",      "ps"  , "" }
};
int numTypes = 9;

enum {
    TYPE_PDF = 0,
    TYPE_PDF_LINEAR,
    TYPE_PDF_13,
//    TYPE_PDF_A1_RGB,
//    TYPE_PDF_A1_CMYK,
//    TYPE_PDF_A2_RGB,
//    TYPE_PDF_A2_CMYK,
//    TYPE_PDF_X3_GRAY,
//    TYPE_PDF_X3_CMYK,
    TYPE_PCL_XL,
    TYPE_XPS,
    TYPE_TEXT,
    TYPE_HTML,
    TYPE_XML,
    TYPE_PS
};

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
        QString dst = dialog.selectedFiles().first();
        QString filter = dialog.selectedNameFilter();
        QString filterExt = QtUtil::extensionFromFilter(filter);
        QString fileExt = QtUtil::extensionFromPath(dst);

        //  if the user gave no extension, use the filter
        if (fileExt.length()==0)
        {
            fileExt = filterExt;
            dst += ".";
            dst += filterExt;
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

        if (index==TYPE_PDF)
        {
            //  regular PDF - just copy the file
            if (QFile::exists(dst))
                QFile::remove(dst);
            QFile::copy(original, dst);
        }
        else if (index==TYPE_PDF_LINEAR)
        {
            //  linearized PDF
            if (QFile::exists(dst))
                QFile::remove(dst);
            m_window->document()->PDFExtract (original.toStdString().c_str(), dst.toStdString().c_str(),
                                              password.toStdString().c_str(),
                                              password.length()>0, true, -1, NULL);
        }
        else if (index==TYPE_PDF_13)
        {
            QString options("-dCompatibilityLevel=1.3 -sDEVICE=pdfwrite -dNOPAUSE -dBATCH");
            saveWithProgress (options, original, dst);
        }
        else if (index==TYPE_XPS)
        {
            QString options("-dNOCACHE -sDEVICE=xpswrite -dNOPAUSE -dBATCH");
            saveWithProgress (options, original, dst);
        }
        else if (index==TYPE_TEXT)
        {
            saveAsText (dst, TEXT);
        }
        else if (index==TYPE_HTML)
        {
            saveAsText (dst, HTML);
        }
        else if (index==TYPE_XML)
        {
            saveAsText (dst, XML);
        }
        else if (index==TYPE_PS)
        {
            QString options("-sDEVICE=ps2write -dNOPAUSE -dBATCH -P- -dSAFER");
            saveWithProgress (options, original, dst);
        }
        else
        {
            //  NYI
            QString message = "saving is not yet implemented.<br/><br/>";
            message += "saving " + dst + "<br/>as " + QString::number(index+1) + ". " + filter;
            QMessageBox::information (m_window, "", message);
        }
    }

}

void FileSave::setProgress (int val)
{
    m_progressDialog->setValue(val);
    QString s = QString("Processed ")
                    + QString::number(val) + QString(" of ")
                    + QString::number(m_window->document()->GetPageCount()) + QString(" pages...");
    m_progressDialog->setLabelText(s);
}

void FileSave::saveAsText(QString dst, int type)
{
    //  determine temp file
    QString tmp = dst + ".temp";

    //  remove previous temp
    if (QFile::exists(tmp))
        QFile::remove(tmp);

    int nPages = m_window->document()->GetPageCount();

    //  show a progress widget
    m_progressDialog = new QProgressDialog(m_window);
//    connect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
    m_progressDialog->setMaximum(m_window->document()->GetPageCount());
    setProgress(0);
    m_progressDialog->show();
    qApp->processEvents();

    //  open the file
    QFile file(tmp);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    if (type==HTML)
    {
        out << QString("<html>\n");
        out << QString("<head>\n");
        out << QString("</head>\n");
        out << QString("<body>\n");
    }

    if (type==XML)
    {
        out << QString("<document>\n");
    }

    //  write pages
    bool canceled = false;
    for (int i=0; i<nPages; i++)
    {
        std::string str = m_window->document()->GetText(i,type);
        QString qstr(str.c_str());
        out << qstr;

        setProgress(i+1);
        qApp->processEvents();

        if (m_progressDialog->wasCanceled())
        {
            canceled = true;
            break;
        }
    }

    if (type==HTML)
    {
        out << QString("</body>\n");
        out << QString("</html>\n");
    }

    if (type==XML)
    {
        out << QString("</document>\n");
    }

    //  close the file
    file.close();

    //  take down progress widget
    m_progressDialog->hide();
    qApp->processEvents();
//    disconnect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
    delete m_progressDialog;

    if (canceled)
    {
        //  remove temp
        if (QFile::exists(tmp))
            QFile::remove(tmp);
        QMessageBox::information (m_window, "", "Canceled.");
    }
    else
    {
        //  remove destination file
        if (QFile::exists(dst))
            QFile::remove(dst);

        //  put temp file in place
        QFile::rename(tmp, dst);

        QMessageBox::information (m_window, "", "Done.");
    }
}

void FileSave::saveWithProgress (QString options, QString src, QString dst)
{
    m_dst = dst;
    m_tmp = dst + ".temp";

    //  construct the command
    QString command = "\"" + QtUtil::getGsPath() + "\"";
    command += " " + options + " ";
//    command += " -r72 ";
    command += " -o \"" + m_tmp + "\"";
    command += " -f \"" + src + "\"";

    MessagesDialog::addMessage("\n");
    MessagesDialog::addMessage("starting\n");
    MessagesDialog::addMessage(command+"\n\n");

    //  show a progress widget
    m_progressDialog = new QProgressDialog(m_window);
    connect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
    m_progressDialog->setMaximum(m_window->document()->GetPageCount());
    setProgress(0);
    m_progressDialog->show();
    qApp->processEvents();

    //  create a process to do the conversion.  During this time onReadyReadStandardOutput gets called;
    //  that's our opportunity to update the progress widget
    m_process = new QProcess(m_window);
    connect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
    connect (m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    m_process->start(command);

    //  don't wait here, that will block the UI.
    //  instead, clean up in onFinshed(), which will be called in both the
    //  regular finish and when cancelling.
}

void FileSave::onReadyReadStandardOutput()
{
    char data[10000];
    while (m_process->canReadLine())
    {
        int nc = m_process->readLine(data, 10000);
        data[nc] = 0;

        //  add to messages
        MessagesDialog::addMessage(QString(data));

        //  every time we see "Page", crank the progress
        QString s(data);
        if (s.left(4).compare(QString("Page"))==0)
        {
            int val = m_progressDialog->value();
            setProgress(val+1);
            qApp->processEvents();
        }
    }
}

void FileSave::onCanceled()
{
    m_process->terminate();
}

void FileSave::onFinished(int exitCode)
{
    UNUSED(exitCode);

    //  are we canceled?
    if (m_progressDialog->wasCanceled())
    {
        //  yes
        MessagesDialog::addMessage("canceled.\n");

        //  remove temp file
        if (QFile::exists(m_tmp))
            QFile::remove(m_tmp);

        QMessageBox::information (NULL, "", "Canceled.");
    }
    else
    {
        //  no
        MessagesDialog::addMessage("finished.\n");

        //  remove destination file
        if (QFile::exists(m_dst))
            QFile::remove(m_dst);

        //  put temp file in place
        QFile::rename(m_tmp, m_dst);

        QMessageBox::information (NULL, "", "Finished.");
    }

    //  take down progress widget
    m_progressDialog->hide();
    qApp->processEvents();
    disconnect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
    delete m_progressDialog;

    //  disconnect/delete the process
    disconnect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
    disconnect (m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
//    delete m_process;
//    m_process=NULL;

}

