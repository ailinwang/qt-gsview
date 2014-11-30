#include "ExtractPagesDialog.h"
#include "ui_ExtractPagesDialog.h"
#include "Window.h"
#include "QtUtil.h"
#include "MessagesDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProcess>
#include <QProgressDialog>

std::vector<device_t> devices = {

    {0,"svg","svg","svg","mupdf","single",SVG_OUT},
    {1,"pnm","pnm","pnm","mupdf","single",PNM_OUT},
    {2,"pclbitmap","pclbitmap","pcl","mupdf","single",PCL_OUT},
    {3,"pwg","pwg","pwg","mupdf","single",PWG_OUT},

    {4,"bmp16","bmp16","bmp","gs","single",0},
    {5,"bmp16m","bmp16m","bmp","gs","single",0},
    {6,"bmp256","bmp256","bmp","gs","single",0},
    {7,"bmp32b","bmp32b","bmp","gs","single",0},
    {8,"bmpgray","bmpgray","bmp","gs","single",0},
    {9,"bmpmono","bmpmono","bmp","gs","single",0},
    {10,"eps2write","eps2write","eps","gs","single",0},
    {11,"jpeg","jpeg","jpg","gs","single",0},
    {12,"jpegcmyk","jpegcmyk","jpg","gs","single",0},
    {13,"jpeggray","jpeggray","jpg","gs","single",0},
    {14,"pamcmyk32","pamcmyk32","pam","gs","single",0},
    {15,"pamcmyk4","pamcmyk4","pam","gs","single",0},
    {16,"pbm","pbm","pbm","gs","single",0},
    {17,"pgm","pgm","pgm","gs","single",0},
    {18,"png16","png16","png","gs","single",0},
    {19,"png16m","png16m","png","gs","single",0},
    {20,"png256","png256","png","gs","single",0},
    {21,"pngalpha","pngalpha","png","gs","single",0},
    {22,"pnggray","pnggray","png","gs","single",0},
    {23,"pngmono","pngmono","png","gs","single",0},
    {24,"psdcmyk","psdcmyk","psd","gs","single",0},

    {25,"psdrgb  ","psdrgb  ","psd","gs","multi",0},
    {26,"pdfwrite","pdfwrite","pdf","gs","multi",0},
    {27,"ps2write","ps2write","ps","gs","multi",0},
    {28,"pxlcolor","pxlcolor","pxl","gs","multi",0},
    {29,"pxlmono","pxlmono","pxl","gs","multi",0},
    {30,"tiff12nc","tiff12nc","tiff","gs","multi",0},
    {31,"tiff24nc","tiff24nc","tiff","gs","multi",0},
    {32,"tiff32nc","tiff32nc","tiff","gs","multi",0},
    {33,"tiff64nc","tiff64nc","tiff","gs","multi",0},
    {34,"tiffcrle","tiffcrle","tiff","gs","multi",0},
    {35,"tiffg3","tiffg3","tiff","gs","multi",0},
    {36,"tiffg32d","tiffg32d","tiff","gs","multi",0},
    {37,"tiffg4","tiffg4","tiff","gs","multi",0},
    {38,"tiffgray","tiffgray","tiff","gs","multi",0},
    {39,"tifflzw","tifflzw","tiff","gs","multi",0},
    {40,"tiffpack","tiffpack","tiff","gs","multi",0},
    {41,"tiffsep","tiffsep","tiff","gs","multi",0},
    {42,"txtwrite","txtwrite","txt","gs","multi",0},
    {43,"xpswrite","xpswrite","xps","gs","multi",0},
};

ExtractPagesDialog::ExtractPagesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtractPagesDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    //  default value for resolution
    ui->resolution->setText("300");
}

ExtractPagesDialog::~ExtractPagesDialog()
{
    delete ui;
}

void ExtractPagesDialog::run(Window *win)
{
    //  remember the window
    m_window = win;

    //  set up page list
    ui->pageList->clear();
    for (int i=0; i<m_window->document()->GetPageCount(); i++)
        ui->pageList->addItem(QString::number(i+1));

    //  set up the format list
    for (int i=0; i<(int)devices.size(); i++)
    {
        device_t device = devices.at(i);
        ui->formatList->addItem(QString(device.label));
    }

    this->setWindowModality(Qt::ApplicationModal);

    show();
}

void ExtractPagesDialog::on_cancelButton_clicked()
{
    close();
}

void ExtractPagesDialog::on_allButton_clicked()
{
    //  select all pages
    for (int i=0; i<m_window->document()->GetPageCount(); i++)
        ui->pageList->item(i)->setSelected(true);
}

void ExtractPagesDialog::on_evenButton_clicked()
{
    //  select even-numbered pages
    for (int i=0; i<m_window->document()->GetPageCount(); i++)
        ui->pageList->item(i)->setSelected(i%2);
}

void ExtractPagesDialog::on_oddButton_clicked()
{
    //  select odd-numbered pages
    for (int i=0; i<m_window->document()->GetPageCount(); i++)
        ui->pageList->item(i)->setSelected(!(i%2));
}

void ExtractPagesDialog::on_noneButton_clicked()
{
    //  select no pages
    for (int i=0; i<m_window->document()->GetPageCount(); i++)
        ui->pageList->item(i)->setSelected(false);
}

void ExtractPagesDialog::on_extractButton_clicked()
{
    //  the actual extraction

    //  must have pages selected
    if (ui->pageList->selectedItems().size()<=0)
    {
        QMessageBox::information(NULL, tr(""), tr("You must select one or more pages."));
        return;
    }

    //  must have a format selected
    if (ui->formatList->selectedItems().size()<=0)
    {
        QMessageBox::information(NULL, tr(""), tr("You must select a device."));
        return;
    }

    //  get selected device
    m_device = devices[ui->formatList->currentIndex().row()];

    //  get resolution and options
    m_options = ui->ghostscriptOptions->toPlainText();
    m_resolution = ui->resolution->text();

    doSave();
}

void ExtractPagesDialog::doSave()
{
    //  if original is cbz and command is gs, that's
    //  currently unsupported.
    if (QtUtil::extensionFromPath(m_window->getPath())==QString("cbz") &&
        m_device.command.compare("gs")==0    )
    {
        QMessageBox::information(NULL, tr(""), tr("Saving pages from CBZ files to that format is not yet supported."));
        return;
    }

    //  where is the desktop?
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QString desktop = desktopLocations.first();

    //  set up the dialog
    QFileDialog dialog(m_window, tr("Save"), desktop);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::DontUseNativeDialog, !USE_NATIVE_FILE_DIALOGS);
    QString theFilter = QString(tr("%1 files (*.%2)")).arg(m_device.label, m_device.extension);
    dialog.setNameFilter(theFilter);

    //  get the name
    dialog.setWindowModality(Qt::ApplicationModal);
    dialog.show();
    int result = dialog.exec();
    if (result == QDialog::Accepted)
    {
        hide();
        m_destination = dialog.selectedFiles().first();

        //  if the user gave no extension, use the filter
        QString fileExt = QtUtil::extensionFromPath(m_destination);
        if (fileExt.length()==0)
        {
            m_destination += ".";
            m_destination += m_device.extension;
        }

        //  check for contiguity
        int on = 0;
        m_firstPage = -1;
        m_lastPage = -1;
        int nsec=0;

        for (int i=0; i<m_window->document()->GetPageCount(); i++)
        {
            if (ui->pageList->item(i)->isSelected())
            {
                if (on==0)
                    nsec++;
                if (m_firstPage==-1)
                    m_firstPage = i+1;
                m_lastPage = i+1;
                on = 1;
            }
            else
            {
                on = 0;
            }
        }
        m_contiguous = (nsec==1);

        //  now do it
        if (m_device.command.compare("mupdf")==0)
        {
            doSaveMupdf();
        }
        else if (m_device.command.compare("gs")==0)
        {
            doSaveGs();
        }
    }
}

void ExtractPagesDialog::doSaveMupdf()
{
    //  TODO:  use a thread

    //  count pages
    int nPages = 0;
    for (int i=0; i<m_window->document()->GetPageCount(); i++)
        if (ui->pageList->item(i)->isSelected())
            nPages++;

    //  show progress
    //  show a progress widget
    m_progressDialog = new QProgressDialog(m_window);
    m_progressDialog->setMaximum(nPages);
    setProgress(0);
    m_progressDialog->show();
    qApp->processEvents();

    //  save a file for each page
    int np = 0;
    for (int i=0; i<m_window->document()->GetPageCount(); i++)
    {
        //  cancelled?
        if (m_progressDialog->wasCanceled())
            break;

        if (ui->pageList->item(i)->isSelected())
        {
            np++;
            setProgress(np);

            //  make a command for this page
            QString page = QString::number(i+1);

            //  make a page-numbered file name
            QFileInfo original(m_destination);
            QString newPath = original.absoluteDir().path() + QDir::separator() +
                    original.baseName() + tr("-page") + page;
            if (!original.completeSuffix().isEmpty())
                newPath += "." + original.completeSuffix();
            else
                newPath += "." + m_device.extension;

            int res = 300;
            if (!m_resolution.isEmpty())
            {
                bool bOK;
                int res2 = m_resolution.toInt(&bOK);
                if (bOK)
                    res = res2;
            }

            MessagesDialog::addMessage("saving page: ");
            MessagesDialog::addMessage(newPath);
            MessagesDialog::addMessage("\n");
            m_window->document()->SavePage((char *)newPath.toStdString().c_str(), i, res, m_device.type, false);
        }
    }

    //  take down progress widget
    m_progressDialog->hide();
    qApp->processEvents();
    delete m_progressDialog;

    if (m_progressDialog->wasCanceled())
    {
        MessagesDialog::addMessage(tr("Extract cancelled."));  MessagesDialog::addMessage("\n");
        QMessageBox::information (NULL, "", tr("Extract cancelled."));
    }
    else
    {
         MessagesDialog::addMessage(tr("Extract completed."));  MessagesDialog::addMessage("\n");
        QMessageBox::information (NULL, "", tr("Extract completed."));
    }

}

void ExtractPagesDialog::doSaveGs()
{
    m_commands.clear();
    m_currentCommand = 0;

    QString originalPath(m_window->getPath());

    //  if the original is xps, convert tp pdf first.
    if (QtUtil::extensionFromPath(originalPath)==QString("xps"))
    {
        //  put the result into the temp folder
        QFileInfo fileInfo (originalPath);
        QString newPath = QtUtil::getTempFolderPath() + fileInfo.fileName() + ".pdf";

        //  construct the command
        QString command = "\"" + QtUtil::getGxpsPath() + "\"";
        command += " -sDEVICE=pdfwrite ";
        command += "-sOutputFile=\"" + newPath + "\"";
        command += " -dNOPAUSE \"" + originalPath + "\"";

        //  create a process to do it, and wait
        QProcess *process = new QProcess(this);
        process->start(command);
        process->waitForFinished();

        originalPath = newPath;
    }

    if (m_contiguous && m_device.paging.compare("multi")==0)
    {
        //  we can do this in one shot

        QString command;
        command = "\"" + QtUtil::getGsPath() + "\"";

        command += " -dFirstPage=" + QString::number(m_firstPage) + " ";
        command += " -dLastPage=" + QString::number(m_lastPage)  + " ";

        if (!m_options.isEmpty())
            command += " " + m_options + " ";
        command += " -dNOPAUSE -dBATCH ";
        command += " -sDEVICE=" + m_device.name + " ";
        if (!m_resolution.isEmpty())
            command += " -r" + m_resolution + " ";

        command += " -o \"" + m_destination + "\"";
        command += " -f \"" + originalPath + "\"";

        m_commands.push_back(command);
    }
    else
    {
        //  save a file for each page
        for (int i=0; i<m_window->document()->GetPageCount(); i++)
        {
            if (ui->pageList->item(i)->isSelected())
            {
                //  make a command for this page
                QString page = QString::number(i+1);

                QString command;
                command = "\"" + QtUtil::getGsPath() + "\"";
                command += " -dFirstPage=" + page + " ";
                command += " -dLastPage=" + page + " ";
                if (!m_options.isEmpty())
                    command += " " + m_options + " ";
                command += " -dNOPAUSE -dBATCH ";
                command += " -sDEVICE=" + m_device.name + " ";
                if (!m_resolution.isEmpty())
                    command += " -r" + m_resolution + " ";

                //  make a page-numbered file name
                QFileInfo original(m_destination);
                QString newPath = original.absoluteDir().path() + QDir::separator() +
                        original.baseName() + "-page" + page;
                if (!original.completeSuffix().isEmpty())
                    newPath += "." + original.completeSuffix();
                else
                    newPath += "." + m_device.extension;
                command += " -o \"" + newPath + "\"";

                command += " -f \"" + originalPath + "\"";

                m_commands.push_back(command);
            }
        }
    }

    doSave2();
}

void ExtractPagesDialog::doSave2()
{
    //  show a progress widget
    m_progressDialog = new QProgressDialog(m_window);
    connect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
    m_progressDialog->setMaximum(m_commands.size());
    setProgress(0);
    m_progressDialog->show();
    qApp->processEvents();

    //  create process to do the commands
    m_process = new QProcess(m_window);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
    connect (m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));

    //  start the first command
    startCommand(m_commands.at(m_currentCommand));
}

void ExtractPagesDialog::onReadyReadStandardOutput()
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
        if (s.left(4).compare(QString("Page"))==0)  //  don't translate this string
        {
            int val = m_progressDialog->value();
            setProgress(val+1);
        }
    }
}

void ExtractPagesDialog::onCanceled()
{
    m_process->terminate();
}

void ExtractPagesDialog::onFinished(int exitCode)
{
    UNUSED(exitCode);

    //  are we canceled?
    if (m_progressDialog->wasCanceled())
    {
        //  yes
        MessagesDialog::addMessage(tr("Extract cancelled."));  MessagesDialog::addMessage("\n");
        QMessageBox::information (NULL, "", tr("Extract cancelled."));
    }
    else
    {
        //  not canceled

        //  more commands?  Keep going
        m_currentCommand++;
        if (m_currentCommand<(int)m_commands.size())
        {
            startCommand(m_commands.at(m_currentCommand));
            return;
        }

        //  we're done
        MessagesDialog::addMessage(tr("Extract completed."));  MessagesDialog::addMessage("\n");
        QMessageBox::information (NULL, "", tr("Extract completed."));
    }

    //  take down progress widget
    m_progressDialog->hide();
    qApp->processEvents();
    disconnect (m_progressDialog, SIGNAL(canceled()), this, SLOT(onCanceled()));
    delete m_progressDialog;

    //  disconnect/delete the process
    disconnect (m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStandardOutput()));
    disconnect (m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));

    close();
}

void ExtractPagesDialog::setProgress (int val)
{
    m_progressDialog->setValue(val);
    QString s = QString(tr("Processed %1 of %2 pages...")).arg(
                QString::number(val), QString::number(m_progressDialog->maximum()));
    m_progressDialog->setLabelText(s);
    qApp->processEvents();
}

void ExtractPagesDialog::startCommand(QString command)
{
    MessagesDialog::addMessage("\n");
    MessagesDialog::addMessage(tr("starting"));
    MessagesDialog::addMessage("\n");
    MessagesDialog::addMessage(command+"\n\n");

    m_process->start(command);
}

