#include "PrintDialog.h"
#include "ui_PrintDialog.h"
#include "Printer.h"

#include <QtPrintSupport/QPrinterInfo>
#include <QDebug>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QMessageBox>
#include <QTimer>

PrintDialog::PrintDialog(QWidget *parent, int maxPages, int currentPage, QPrinter *printer, QString path) :
    QDialog(parent),
    ui(new Ui::PrintDialog),
    m_path(path)
{
    //  set up the UI
    ui->setupUi(this);

    //  set the window title
    setWindowTitle(tr("Print Document"));

    //  insure we've got the arrow cursor.
    qApp->setOverrideCursor(Qt::ArrowCursor);
    qApp->processEvents();

    //  start with all pages
    ui->allRadioButton->setChecked(true);

    //  fill in printer names
    m_printerList=QPrinterInfo::availablePrinters();
    foreach (QPrinterInfo printerInfo, m_printerList)
        ui->printerCombo->addItem(printerInfo.description());

    //  printer
    m_printer = printer;

    //  document (for rendering)
    m_document = new Document();
    m_document->Initialize();
    m_document->OpenFile(m_path.toStdString().c_str());

    //  set up slider based on the number of pages
    m_currentPage = currentPage;
    m_maxPages = maxPages;
    setupSlider();
}

PrintDialog::~PrintDialog()
{
    delete ui;
}

QString PrintDialog::printRange()
{
    if (ui->allRadioButton->isChecked())
    {
        //  all pages
        return QString("1-%1").arg(m_maxPages);
    }
    else if (ui->currentRadioButton->isChecked())
    {
        //  current page
        return QString("%1-%1").arg(m_currentPage);
    }
    else if (ui->pagesRadioButton->isChecked())
    {
        //  page range
        return ui->pageListEdit->toPlainText();
    }

    return QString("");
}

int PrintDialog::copies()
{
    return ui->copiesSpinner->value();
}

void PrintDialog::on_propertiesButton_clicked()
{
    //  user clicked properties button

    //  get info for the currently selected printer
    int index = ui->printerCombo->currentIndex();
    QPrinterInfo printerInfo = m_printerList.at(index);

    //  not sure how to resolve this just yet.
    //  on OSX, the page setup dialog has a
    //  "format for" menu.  It seems to be populated with the last-selected
    //  printer in the OS, *not* the one selected in this dialog.

    m_printer->setPrinterName(printerInfo.printerName());
    QPageSetupDialog dlg(m_printer);
    if (dlg.exec() == QDialog::Accepted)
    {
        //  nothing to do?
    }

}

void PrintDialog::on_cancelButton_clicked()
{
    //  user clicked cancel

    //  close file
    if (m_document != NULL)
    {
        m_document->CleanUp();
        delete m_document;
        m_document = NULL;
    }

    //  restore cursor
    QApplication::restoreOverrideCursor();
    qApp->processEvents();

    QDialog::reject();
}

void PrintDialog::on_printButton_clicked()
{
    //  user clicked print button

    //  first check to see if the range is valid
    QString range = this->printRange();
    QList<int> pageList = Printer::listFromRange(range, m_maxPages);
    if (pageList.empty())
    {
        QMessageBox::information(this, "", tr("Invalid page range."));
        return;
    }

    //  close file
    if (m_document != NULL)
    {
        m_document->CleanUp();
        delete m_document;
        m_document = NULL;
    }

    //  set selected printer
    m_printer->setPrinterName(m_printerList.at(ui->printerCombo->currentIndex()).printerName());

    //  restore cursor
    QApplication::restoreOverrideCursor();
    qApp->processEvents();

    QDialog::accept();
}

void PrintDialog::on_allRadioButton_clicked()
{
    //  user clicked all pages radio
    setupSlider();
    updatePreview();
}

void PrintDialog::on_currentRadioButton_clicked()
{
    //  user clicked current page radio
    setupSlider();
    updatePreview();
}

void PrintDialog::on_pagesRadioButton_clicked()
{
    //  user clicked pages radio
    setupSlider();
    updatePreview();
}

void PrintDialog::on_pageListEdit_textChanged()
{
    //  user is typing page numbers, so select the radio button
    ui->pagesRadioButton->setChecked(true);
    setupSlider();
    updatePreview();
}

void PrintDialog::on_printerCombo_currentIndexChanged(int index)
{
    //  user changed selected printer
}

void PrintDialog::setSliderLabel(int val)
{
    QString range = this->printRange();
    QList<int> pageList = Printer::listFromRange(range, m_maxPages);
    int np = pageList.size();

    ui->pageSliderLabel->setText(QString::number(val) + "/" + QString::number(np));
}

void PrintDialog::setupSlider()
{
    QString range = this->printRange();
    QList<int> pageList = Printer::listFromRange(range, m_maxPages);
    int np = pageList.size();

    if (np<=1)
    {
        ui->pageSlider->setRange(1, 2);
        ui->pageSlider->setEnabled(false);
    }
    else
    {
        ui->pageSlider->setRange(1, np);
        ui->pageSlider->setEnabled(true);
    }

    setSliderLabel(1);
}

void PrintDialog::updatePreview()
{
    if (m_timer == NULL)
    {
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(onPreviewTimer()));
    }

    m_timer->stop();
    m_timer->start(200);
}

void PrintDialog::onPreviewTimer()
{
    renderPreview();
}

void PrintDialog::renderPreview()
{
    ui->previewLabel->clear();

    //  page number
    QString range = printRange();
    QList<int> pageList = Printer::listFromRange(range, m_maxPages);
    if (pageList.isEmpty())
        return;
    int index = ui->pageSlider->value()-1;
    int pageNumber = pageList.at(index) - 1;

    //  delete previous image data
    if (m_image!=NULL)  delete m_image;  m_image=NULL;
    if (m_bitmap!=NULL) delete m_bitmap; m_bitmap=NULL;

    //  get dimensions of the widget
    int w = ui->previewLabel->width();
    int h = ui->previewLabel->height();

    //  figure out a scale factor
    point_t pageSize;
    m_document->GetPageSize(pageNumber, 1.0, &pageSize);
    double scaleh = double(h)/pageSize.Y;
    double scalew = double(w)/pageSize.X;
    double scale = fmin(scaleh, scalew);

    //  render
    int numBytes = w * h * 4;
    m_bitmap = new Byte[numBytes];
    m_document->RenderPage (pageNumber, scale, m_bitmap, w, h, false);

    //  copy to widget
    m_image = new QImage(m_bitmap, w, h, QImage::Format_ARGB32);
    m_pixmap = QPixmap::fromImage(*m_image);
    ui->previewLabel->setPixmap(m_pixmap);
    ui->previewLabel->update();
}

void PrintDialog::on_pageSlider_valueChanged(int value)
{
    setSliderLabel(value);
    updatePreview();
}
