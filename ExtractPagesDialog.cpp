#include "ExtractPagesDialog.h"
#include "ui_ExtractPagesDialog.h"
#include "Window.h"

#include <QMessageBox>

class device_t
{
public:
    int index;
    QString name;
    QString label;
};

std::vector<device_t> devices = {
    {0,"svg","svg"},
    {1,"pnm","pnm"},
    {2,"pclbitmap","pclbitmap"},
    {3,"pwg","pwg"},
    {4,"bmp16","bmp16"},            /* Add mupdf devices before this one */
    {5,"bmp16m","bmp16m"},
    {6,"bmp256","bmp256"},
    {7,"bmp32b","bmp32b"},
    {8,"bmpgray","bmpgray"},
    {9,"bmpmono","bmpmono"},
    {10,"eps2write","eps2write"},
    {11,"jpeg","jpeg"},
    {12,"jpegcmyk","jpegcmyk"},
    {13,"jpeggray","jpeggray"},
    {14,"pamcmyk32","pamcmyk32"},
    {15,"pamcmyk4","pamcmyk4"},
    {16,"pbm","pbm"},
    {17,"pgm","pgm"},
    {18,"png16","png16"},
    {19,"png16m","png16m"},
    {20,"png256","png256"},
    {21,"pngalpha","pngalpha"},
    {22,"pnggray","pnggray"},
    {23,"pngmono","pngmono"},
    {24,"psdcmyk","psdcmyk"},
    {25,"psdrgb  ","psdrgb  "},         /* Add single page gs devices before this device */
    {26,"pdfwrite","pdfwrite"},
    {27,"ps2write","ps2write"},
    {28,"pxlcolor","pxlcolor"},
    {29,"pxlmono","pxlmono"},
    {30,"tiff12nc","tiff12nc"},
    {31,"tiff24nc","tiff24nc"},
    {32,"tiff32nc","tiff32nc"},
    {33,"tiff64nc","tiff64nc"},
    {34,"tiffcrle","tiffcrle"},
    {35,"tiffg3","tiffg3"},
    {36,"tiffg32d","tiffg32d"},
    {37,"tiffg4","tiffg4"},
    {38,"tiffgray","tiffgray"},
    {39,"tifflzw","tifflzw"},
    {40,"tiffpack","tiffpack"},
    {41,"tiffsep","tiffsep"},
    {42,"txtwrite","txtwrite"},
    {43,"xpswrite","xpswrite"},
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

    //  get resolution and options
    m_options = ui->ghostscriptOptions->toPlainText();
    m_resolution = ui->resolution->text();
}
