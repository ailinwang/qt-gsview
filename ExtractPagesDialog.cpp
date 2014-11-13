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

    {0,"svg","svg","svg","mupdf","single"},
    {1,"pnm","pnm","pnm","mupdf","single"},
    {2,"pclbitmap","pclbitmap","pcl","mupdf","single"},
    {3,"pwg","pwg","pwg","mupdf","single"},
    {4,"bmp16","bmp16","bmp","gs","single"},
    {5,"bmp16m","bmp16m","bmp","gs","single"},
    {6,"bmp256","bmp256","bmp","gs","single"},
    {7,"bmp32b","bmp32b","bmp","gs","single"},
    {8,"bmpgray","bmpgray","bmp","gs","single"},
    {9,"bmpmono","bmpmono","bmp","gs","single"},
    {10,"eps2write","eps2write","eps","gs","single"},
    {11,"jpeg","jpeg","jpg","gs","single"},
    {12,"jpegcmyk","jpegcmyk","jpg","gs","single"},
    {13,"jpeggray","jpeggray","jpg","gs","single"},
    {14,"pamcmyk32","pamcmyk32","pam","gs","single"},
    {15,"pamcmyk4","pamcmyk4","pam","gs","single"},
    {16,"pbm","pbm","pbm","gs","single"},
    {17,"pgm","pgm","pgm","gs","single"},
    {18,"png16","png16","png","gs","single"},
    {19,"png16m","png16m","png","gs","single"},
    {20,"png256","png256","png","gs","single"},
    {21,"pngalpha","pngalpha","png","gs","single"},
    {22,"pnggray","pnggray","png","gs","single"},
    {23,"pngmono","pngmono","png","gs","single"},
    {24,"psdcmyk","psdcmyk","psd","gs","single"},
    {25,"psdrgb  ","psdrgb  ","psd","gs","multi"},
    {26,"pdfwrite","pdfwrite","pdf","gs","multi"},
    {27,"ps2write","ps2write","ps","gs","multi"},
    {28,"pxlcolor","pxlcolor","pxl","gs","multi"},
    {29,"pxlmono","pxlmono","pxl","gs","multi"},
    {30,"tiff12nc","tiff12nc","tiff","gs","multi"},
    {31,"tiff24nc","tiff24nc","tiff","gs","multi"},
    {32,"tiff32nc","tiff32nc","tiff","gs","multi"},
    {33,"tiff64nc","tiff64nc","tiff","gs","multi"},
    {34,"tiffcrle","tiffcrle","tiff","gs","multi"},
    {35,"tiffg3","tiffg3","tiff","gs","multi"},
    {36,"tiffg32d","tiffg32d","tiff","gs","multi"},
    {37,"tiffg4","tiffg4","tiff","gs","multi"},
    {38,"tiffgray","tiffgray","tiff","gs","multi"},
    {39,"tifflzw","tifflzw","tiff","gs","multi"},
    {40,"tiffpack","tiffpack","tiff","gs","multi"},
    {41,"tiffsep","tiffsep","tiff","gs","multi"},
    {42,"txtwrite","txtwrite","txt","gs","multi"},
    {43,"xpswrite","xpswrite","xps","gs","multi"},
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

//    //  from 4-25, can only do one page
//    if (ui->pageList->selectedItems().size()>1)
//    {
//        if (m_device.index>=4 && m_device.index<=25)
//        {
//            QMessageBox::information(NULL, tr(""), tr("You can only extract one page at a time with the selected device."));
//            return;
//        }
//    }

    //  TODO: no mupdf yet
    if (m_device.command.compare("mupdf")==0)
    {
        QMessageBox::information(NULL, tr(""), tr("That device is not yet implemented"));
        return;
    }

    //  get resolution and options
    m_options = ui->ghostscriptOptions->toPlainText();
    m_resolution = ui->resolution->text();

    doSave();
}

void ExtractPagesDialog::doSave()
{
    //  where is the desktop?
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QString desktop = desktopLocations.first();

    //  set up the dialog
    QFileDialog dialog(m_window, "Save", desktop);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::DontUseNativeDialog, !USE_NATIVE_FILE_DIALOGS);
    QString theFilter = m_device.label + QString(" files (*.") + m_device.extension + QString(")");
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
    QMessageBox::information (NULL, "", "Not yet implemented.");
}

void ExtractPagesDialog::doSaveGs()
{
    m_commands.clear();
    m_currentCommand = 0;

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

            command += " -f \"" + m_window->getPath() + "\"";

            m_commands.push_back(command);
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
        if (s.left(4).compare(QString("Page"))==0)
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
        MessagesDialog::addMessage("canceled.\n");
        QMessageBox::information (NULL, "", "Canceled.");
    }
    else
    {
        //  not canceled

        //  more commands?  Keep going
        m_currentCommand++;
        if (m_currentCommand<m_commands.size())
        {
            startCommand(m_commands.at(m_currentCommand));
            return;
        }

        //  we're done
        MessagesDialog::addMessage("finished.\n");
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

    close();
}

void ExtractPagesDialog::setProgress (int val)
{
    int total = m_commands.size();

    m_progressDialog->setValue(val);
    QString s = QString("Processed ")
                    + QString::number(val) + QString(" of ")
                    + QString::number(total) + QString(" pages...");
    m_progressDialog->setLabelText(s);
    qApp->processEvents();
}

void ExtractPagesDialog::startCommand(QString command)
{
    MessagesDialog::addMessage("\n");
    MessagesDialog::addMessage("starting\n");
    MessagesDialog::addMessage(command+"\n\n");

    m_process->start(command);
}

