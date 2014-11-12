#include "ExtractPagesDialog.h"
#include "ui_ExtractPagesDialog.h"
#include "Window.h"

#include <QMessageBox>

std::vector<std::string> devices = {
    "svg",
    "pnm",
    "pclbitmap",
    "pwg",
    "bmp16",  /* Add mupdf devices before this one */
    "bmp16m",
    "bmp256",
    "bmp32b",
    "bmpgray",
    "bmpmono",
    "eps2write",
    "jpeg",
    "jpegcmyk",
    "jpeggray",
    "pamcmyk32",
    "pamcmyk4",
    "pbm",
    "pgm",
    "png16",
    "png16m",
    "png256",
    "pngalpha",
    "pnggray",
    "pngmono",
    "psdcmyk",
    "psdrgb",  /* Add single page gs devices before this device */
    "pdfwrite",
    "ps2write",
    "pxlcolor",
    "pxlmono",
    "tiff12nc",
    "tiff24nc",
    "tiff32nc",
    "tiff64nc",
    "tiffcrle",
    "tiffg3",
    "tiffg32d",
    "tiffg4",
    "tiffgray",
    "tifflzw",
    "tiffpack",
    "tiffsep",
    "txtwrite",
    "xpswrite"
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
        std::string device = devices.at(i);
        ui->formatList->addItem(QString(device.c_str()));
    }

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
        QMessageBox::information(NULL, tr(""), tr("You must select a format."));
        return;
    }

}
