#include "ExtractPagesDialog.h"
#include "ui_ExtractPagesDialog.h"
#include "Window.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

std::vector<device_t> devices = {

    {0,"svg","svg","svg"},
    {1,"pnm","pnm","pnm"},
    {2,"pclbitmap","pclbitmap","pcl"},
    {3,"pwg","pwg","pwg"},
    {4,"bmp16","bmp16","bmp"},              /* Add mupdf devices before this one */
    {5,"bmp16m","bmp16m","bmp"},
    {6,"bmp256","bmp256","bmp"},
    {7,"bmp32b","bmp32b","bmp"},
    {8,"bmpgray","bmpgray","bmp"},
    {9,"bmpmono","bmpmono","bmp"},
    {10,"eps2write","eps2write","eps"},
    {11,"jpeg","jpeg","jpg"},
    {12,"jpegcmyk","jpegcmyk","jpg"},
    {13,"jpeggray","jpeggray","jpg"},
    {14,"pamcmyk32","pamcmyk32","pam"},
    {15,"pamcmyk4","pamcmyk4","pam"},
    {16,"pbm","pbm","pbm"},
    {17,"pgm","pgm","pgm"},
    {18,"png16","png16","png"},
    {19,"png16m","png16m","png"},
    {20,"png256","png256","png"},
    {21,"pngalpha","pngalpha","png"},
    {22,"pnggray","pnggray","png"},
    {23,"pngmono","pngmono","png"},
    {24,"psdcmyk","psdcmyk","psd"},
    {25,"psdrgb  ","psdrgb  ","psd"},               /* Add single page gs devices before this device */
    {26,"pdfwrite","pdfwrite","pdf"},
    {27,"ps2write","ps2write","ps"},
    {28,"pxlcolor","pxlcolor","pxl"},
    {29,"pxlmono","pxlmono","pxl"},
    {30,"tiff12nc","tiff12nc","tiff"},
    {31,"tiff24nc","tiff24nc","tiff"},
    {32,"tiff32nc","tiff32nc","tiff"},
    {33,"tiff64nc","tiff64nc","tiff"},
    {34,"tiffcrle","tiffcrle","tiff"},
    {35,"tiffg3","tiffg3","tiff"},
    {36,"tiffg32d","tiffg32d","tiff"},
    {37,"tiffg4","tiffg4","tiff"},
    {38,"tiffgray","tiffgray","tiff"},
    {39,"tifflzw","tifflzw","tiff"},
    {40,"tiffpack","tiffpack","tiff"},
    {41,"tiffsep","tiffsep","tiff"},
    {42,"txtwrite","txtwrite","txt"},
    {43,"xpswrite","xpswrite","xps"},


};

ExtractPagesDialog::ExtractPagesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtractPagesDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
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

    //  from 4-25, can only do one page
    if (ui->pageList->selectedItems().size()>1)
    {
        if (m_device.index>=4 && m_device.index<=25)
        {
            QMessageBox::information(NULL, tr(""), tr("You can only extract one page at a time with the selected device."));
            return;
        }
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
        m_destination = dialog.selectedFiles().first();

        if (m_device.index<4)
        {
            doSaveMupdf();
        }
        else
        {
            doSaveGs();
        }
    }
}

void ExtractPagesDialog::doSaveMupdf()
{

}


void ExtractPagesDialog::doSaveGs()
{

}
