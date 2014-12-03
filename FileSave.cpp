#include "FileSave.h"

#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QProgressDialog>
#include <QTemporaryFile>
#include <QtPrintSupport>

#include "QtUtil.h"
#include "MessagesDialog.h"
#include "ICCDialog2.h"
#include "FileSaveDialog.h"

FileType fileTypes[] = {
    {"PDF",             "pdf" , "", false },
    {"Linearized PDF",  "pdf" , "", false },
    {"PDF 1.3",         "pdf" , "", false },
    {"PCL-XL",          "pcl" , "", false },
    {"XPS",             "xps" , "", false },
    {"Text",            "txt" , "", false },
    {"HTML",            "html", "", false },
    {"XML",             "xml" , "", false },
    {"PostScript",      "ps"  , "", false },
    {"",                ""    , "", false },
    {"PDF/A-1 RGB",     "pdf" , "", true  },
    {"PDF/A-1 CMYK",    "pdf" , "", true  },
    {"PDF/A-2 RGB",     "pdf" , "", true  },
    {"PDF/A-2 CMYK",    "pdf" , "", true  },
    {"PDF/X-3 Gray",    "pdf" , "", true  },
    {"PDF/X-3 CMYK",    "pdf" , "", true  },
};
int numTypes = 16;

enum {
    TYPE_PDF = 0,
    TYPE_PDF_LINEAR,
    TYPE_PDF_13,
    TYPE_PCL_XL,
    TYPE_XPS,
    TYPE_TEXT,
    TYPE_HTML,
    TYPE_XML,
    TYPE_PS,
    TYPE_BLANK,
    TYPE_PDF_A1_RGB,
    TYPE_PDF_A1_CMYK,
    TYPE_PDF_A2_RGB,
    TYPE_PDF_A2_CMYK,
    TYPE_PDF_X3_GRAY,
    TYPE_PDF_X3_CMYK,
};

void FileSave::run()
{
    QString original = m_window->getPath();

    //  if the original is CBZ, convert to PDF first?
    if (QtUtil::extensionFromPath(original)==QString("cbz"))
    {
        QMessageBox::information (m_window, "", tr("Saving CBZ files is not yet supported."));
        return;
    }

    //  where is the desktop?
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QString desktop = desktopLocations.first();

    //  set up the dialog
    FileSaveDialog dialog(m_window, tr("Save"), desktop);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::DontUseNativeDialog, !USE_NATIVE_FILE_DIALOGS);

    //  what are the file types?
    QString sep("--------------------");
    QString types;
    int i;
    for (i=0;i<numTypes;i++)
    {
        QString theFilter;
        if (i==TYPE_BLANK)
            theFilter = sep;
        else
            theFilter = QString(tr("%1 (*.%2)")).arg(fileTypes[i].filterName, fileTypes[i].filterType);
        fileTypes[i].filter = theFilter;

        if (i==0)
            dialog.setFallbackFilter(theFilter);

        if (i>0)
            types += ";;";

        types += theFilter;
    }
    dialog.setSeparatorFilter(sep);
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
            QMessageBox::information (m_window, "",
                                      tr("You supplied a filename extension that does not match the format."));
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

        //  get current profiles
        QString rgbProfile  = ICCDialog2::rgbProfile();
        QString cmykProfile = ICCDialog2::cmykProfile();
        QString grayProfile = ICCDialog2::grayProfile();

        //  see if we need to ask for one
        bool askProfiles = false;
        if (rgbProfile.isEmpty())
            if (index==TYPE_PDF_A1_RGB||index==TYPE_PDF_A2_RGB)
                askProfiles = true;
        if (cmykProfile.isEmpty())
            if (index==TYPE_PDF_A1_CMYK||index==TYPE_PDF_A2_CMYK||index==TYPE_PDF_X3_CMYK)
                askProfiles = true;
        if (grayProfile.isEmpty())
            if (index==TYPE_PDF_X3_GRAY)
                askProfiles = true;

        //  ask for profiles
        if (askProfiles)
        {
            ICCDialog2 icc_dialog;
            icc_dialog.show();
            icc_dialog.exec();

            //  get the profile paths from the dialog
            rgbProfile  = icc_dialog.rgbProfile();
            cmykProfile = icc_dialog.cmykProfile();
            grayProfile = icc_dialog.grayProfile();
        }

        //  check again for profiles.
        //  if the one we need is missing, fail.
        bool profileMissing = false;
        if (rgbProfile.isEmpty())
            if (index==TYPE_PDF_A1_RGB||index==TYPE_PDF_A2_RGB)
                profileMissing = true;
        if (cmykProfile.isEmpty())
            if (index==TYPE_PDF_A1_CMYK||index==TYPE_PDF_A2_CMYK||index==TYPE_PDF_X3_CMYK)
                profileMissing = true;
        if (grayProfile.isEmpty())
            if (index==TYPE_PDF_X3_GRAY)
                profileMissing = true;
        if (profileMissing)
        {
            QMessageBox::information (m_window, "",
                                      tr("You have not chosen a profile to use with this output format."));
            return;
        }

        //  now do the save

        //  in all cases, the file dialog has asked the user about overwriting,
        //  so it's OK not to ask here.

        QString password = m_window->password();

        //  if the original is xps, convert tp pdf first.
        if (QtUtil::extensionFromPath(original)==QString("xps"))
        {
            //  put the result into the temp folder
            QFileInfo fileInfo (original);
            QString newPath = QtUtil::getTempFolderPath() + fileInfo.fileName() + ".pdf";

            //  construct the command
            QString command = "\"" + QtUtil::getGxpsPath() + "\"";
            command += " -sDEVICE=pdfwrite ";
            command += "-sOutputFile=\"" + newPath + "\"";
            command += " -dNOPAUSE \"" + original + "\"";

            //  create a process to do it, and wait
            QProcess *process = new QProcess(this);
            process->start(command);
            process->waitForFinished();

            original = newPath;
        }

        if (index==TYPE_PDF)
        {
            //  regular PDF - just copy the file
            if (QFile::exists(dst))
                QFile::remove(dst);
            QFile::copy(original, dst);
            QMessageBox::information (NULL, "", tr("Save completed."));
        }
        else if (index==TYPE_PDF_LINEAR)
        {
            //  linearized PDF
            if (QFile::exists(dst))
                QFile::remove(dst);
            m_window->document()->PDFExtract (original.toStdString().c_str(), dst.toStdString().c_str(),
                                              password.toStdString().c_str(),
                                              password.length()>0, true, -1, NULL);
            QMessageBox::information (NULL, "", tr("Save completed."));
        }
        else if (index==TYPE_PDF_13)
        {
            QString options("-dCompatibilityLevel=1.3 -sDEVICE=pdfwrite -dNOPAUSE -dBATCH");
            saveWithProgress (options, original, dst, password);
        }
        else if (index==TYPE_XPS)
        {
            QString options("-dNOCACHE -sDEVICE=xpswrite -dNOPAUSE -dBATCH");
            saveWithProgress (options, original, dst, password);
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
        else if (index==TYPE_PCL_XL)
        {
            QString options("-sDEVICE=pxlcolor -dNOPAUSE -dBATCH -P- -dSAFER");
            saveWithProgress (options, original, dst, password);

        }
        else if (index==TYPE_PS)
        {
            QString options("-sDEVICE=ps2write -dNOPAUSE -dBATCH -P- -dSAFER");
            saveWithProgress (options, original, dst, password);
        }

        else if (index==TYPE_PDF_A1_RGB)
        {
            QString options("-sDEVICE=pdfwrite -dNOPAUSE -dBATCH -P- -dSAFER -dPDFA=1 -dNOOUTERSAVE -dPDFACompatibilityPolicy=1 -sProcessColorModel=DeviceRGB -dColorConversionStrategy=/RGB -sOutputICCProfile=");
            options += QString("\"");  options += rgbProfile;  options += QString("\"");
            saveWithProgress (options, original, dst, password);

        }
        else if (index==TYPE_PDF_A1_CMYK)
        {
            QString options("-sDEVICE=pdfwrite -dNOPAUSE -dBATCH -P- -dSAFER -dPDFA=1 -dNOOUTERSAVE -dPDFACompatibilityPolicy=1 -sProcessColorModel=DeviceCMYK -dColorConversionStrategy=/CMYK -sOutputICCProfile=");
            options += QString("\"");  options += cmykProfile;  options += QString("\"");
            saveWithProgress (options, original, dst, password);

        }
        else if (index==TYPE_PDF_A2_RGB)
        {
            QString options("-sDEVICE=pdfwrite -dNOPAUSE -dBATCH -P- -dSAFER -dPDFA=2 -dNOOUTERSAVE -dPDFACompatibilityPolicy=1 -sProcessColorModel=DeviceRGB -dColorConversionStrategy=/RGB -sOutputICCProfile=");
            options += QString("\"");  options += rgbProfile;  options += QString("\"");
            saveWithProgress (options, original, dst, password);

        }
        else if (index==TYPE_PDF_A2_CMYK)
        {
            QString options("-sDEVICE=pdfwrite -dNOPAUSE -dBATCH -P- -dSAFER -dPDFA=2 -dNOOUTERSAVE -dPDFACompatibilityPolicy=1 -sProcessColorModel=DeviceCMYK -dColorConversionStrategy=/CMYK -sOutputICCProfile=");
            options += QString("\"");  options += cmykProfile;  options += QString("\"");
            saveWithProgress (options, original, dst, password);

        }
        else if (index==TYPE_PDF_X3_GRAY)
        {
            QString options("-sDEVICE=pdfwrite -dNOPAUSE -dBATCH -P- -dSAFER -dPDFX -dNOOUTERSAVE -dPDFACompatibilityPolicy=1 -sProcessColorModel=DeviceGray -dColorConversionStrategy=/Gray -sOutputICCProfile=");
            options += QString("\"");  options += grayProfile;  options += QString("\"");
            saveWithProgress (options, original, dst, password);
        }
        else if (index==TYPE_PDF_X3_CMYK)
        {
            QString options("-sDEVICE=pdfwrite -dNOPAUSE -dBATCH -P- -dSAFER -dPDFX -dNOOUTERSAVE -dPDFACompatibilityPolicy=1 -sProcessColorModel=DeviceCMYK -dColorConversionStrategy=/CMYK -sOutputICCProfile=");
            options += QString("\"");  options += cmykProfile;  options += QString("\"");
            saveWithProgress (options, original, dst, password);
        }

        else
        {
            //  NYI
            QString message(tr("Saving this file type is not yet implemented.<br/><br/>"));
            message += QString(tr("saving %1<br/>as %2.%3")).arg(dst, QString::number(index+1), filter);
            QMessageBox::information (m_window, "", message);
        }
    }

}

void FileSave::extractSelection(int x, int y, int w, int h, int pageNumber, int resolution)
{
    QString original = m_window->getPath();

    //  if the original is CBZ, convert to PDF first?
    if (QtUtil::extensionFromPath(original)==QString("cbz"))
    {
        QMessageBox::information (m_window, "", tr("Saving CBZ files is not yet supported."));
        return;
    }

    //  where is the desktop?
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QString desktop = desktopLocations.first();

    //  set up the dialog
    FileSaveDialog dialog(m_window, tr("Save"), desktop);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::DontUseNativeDialog, !USE_NATIVE_FILE_DIALOGS);

    //  what are the file types?
    QString types;
    types += "PDF (*.pdf)";
    types += ";;EPS (*.eps)";
    types += ";;PS (*.ps)";
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
            QMessageBox::information (m_window, "",
                                      tr("You supplied a filename extension that does not match the format."));
            return;
        }

        //  if the original is xps, convert tp pdf first.
        if (QtUtil::extensionFromPath(original)==QString("xps"))
        {
            //  put the result into the temp folder
            QFileInfo fileInfo (original);
            QString newPath = QtUtil::getTempFolderPath() + fileInfo.fileName() + ".pdf";

            //  construct the command
            QString command = "\"" + QtUtil::getGxpsPath() + "\"";
            command += " -sDEVICE=pdfwrite ";
            command += "-sOutputFile=\"" + newPath + "\"";
            command += " -dNOPAUSE \"" + original + "\"";

            //  create a process to do it, and wait
            QProcess *process = new QProcess(this);
            process->start(command);
            process->waitForFinished();

            original = newPath;
        }

        //  set the output device based on the file type
        QString device;
        if (filterExt.compare("pdf",Qt::CaseInsensitive)==0)
            device = QString("pdfwrite");
        if (filterExt.compare("eps",Qt::CaseInsensitive)==0)
            device = QString("eps2write");
        if (filterExt.compare("ps",Qt::CaseInsensitive)==0)
            device = QString("ps2write");

        m_dst = dst;
        m_tmp = dst + ".temp";

        QString password = m_window->password();

        //  construct the command
        QString command = "\"" + QtUtil::getGsPath() + "\"";
        command += " " + QString("-sDEVICE=") + QString(device) ;
        command += " " + QString("-dNOPAUSE -dBATCH -P- -dSAFER") ;
        command += " -r" + QString::number(resolution) + " ";
        command += " " + QString("-dFirstPage=") + QString::number(pageNumber+1) + " ";
        command += " " + QString("-dLastPage=") + QString::number(pageNumber+1) + " ";
        command += " " + QString("-dDEVICEWIDTHPOINTS=") + QString::number(w) + " ";
        command += " " + QString("-dDEVICEHEIGHTPOINTS=") + QString::number(h) + " ";
        command += " " + QString("-dFIXEDMEDIA") + " ";
        if (!password.isEmpty())
            command += " -sPDFPassword=" + password + " ";
        command += " -o \"" + m_tmp + "\"";
        command += " " + QString("-c") + " ";
        command += "\"<</Install {";
        command += "-" + QString::number(x) + ".0 " ;
        command += "-" + QString::number(y) + ".0 " ;
        command += "translate (testing) == flush}>> setpagedevice\"";
        command += " -f \"" + original + "\"";

        //  launch it
        saveWithProgress2(command);
    }
}

void FileSave::setProgress (int val)
{
    m_progressDialog->setValue(val);
    QString s = QString(tr("Processed %1 of %2 pages...")).arg(
                QString::number(val), QString::number(m_progressDialog->maximum()));
    m_progressDialog->setLabelText(s);
    qApp->processEvents();
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
        out << QString::fromStdString(str);

        setProgress(i+1);

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
    delete m_progressDialog;

    if (canceled)
    {
        //  remove temp
        if (QFile::exists(tmp))
            QFile::remove(tmp);
        QMessageBox::information (m_window, "", tr("Save cancelled."));
    }
    else
    {
        //  remove destination file
        if (QFile::exists(dst))
            QFile::remove(dst);

        //  put temp file in place
        QFile::rename(tmp, dst);

        QMessageBox::information (m_window, "", tr("Save completed."));
    }
}

void FileSave::saveWithProgress2(QString command)
{
    MessagesDialog::addMessage("\n");
    MessagesDialog::addMessage(tr("starting"));  MessagesDialog::addMessage("\n");
    MessagesDialog::addMessage(command+"\n\n");

    //  show a progress widget
    m_progressDialog = new QProgressDialog(m_window);
    connect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
    m_progressDialog->setMaximum(1);  //  only one page
    setProgress(0);
    m_progressDialog->show();
    qApp->processEvents();

    //  create a process to do the conversion.  During this time onReadyReadStandardOutput gets called;
    //  that's our opportunity to update the progress widget
    m_process = new QProcess(m_window);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
    connect (m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    m_process->start(command);

    //  don't wait here, that will block the UI.
    //  instead, clean up in onFinshed(), which will be called in both the
    //  regular finish and when cancelling.
}

void FileSave::saveWithProgress (QString options, QString src, QString dst, QString password)
{
    m_dst = dst;
    m_tmp = dst + ".temp";

    //  construct the command
    QString command = "\"" + QtUtil::getGsPath() + "\"";
    command += " " + options + " ";
    if (!password.isEmpty())
        command += " -sPDFPassword=" + password + " ";
    command += " -o \"" + m_tmp + "\"";
    command += " -f \"" + src + "\"";

    saveWithProgress2 (command);
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
        //  take down progress widget
        m_progressDialog->hide(); qApp->processEvents();
        disconnect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
        delete m_progressDialog;
        disconnect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
        disconnect (m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));

        //  yes
        MessagesDialog::addMessage(tr("Save cancelled."));  MessagesDialog::addMessage("\n");

        //  remove temp file
        if (QFile::exists(m_tmp))
            QFile::remove(m_tmp);

        QMessageBox::information (NULL, "", tr("Save cancelled."));
    }
    else
    {
        //  take down progress widget
        m_progressDialog->hide(); qApp->processEvents();
        disconnect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
        delete m_progressDialog;
        disconnect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
        disconnect (m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));

        //  no
        MessagesDialog::addMessage(tr("Save completed."));  MessagesDialog::addMessage("\n");

        //  remove destination file
        if (QFile::exists(m_dst))
            QFile::remove(m_dst);

        //  put temp file in place
        QFile::rename(m_tmp, m_dst);

        QMessageBox::information (NULL, "", tr("Save completed."));
    }

}
