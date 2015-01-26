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
    ui->printerCombo->blockSignals(true);
    m_printerList=QPrinterInfo::availablePrinters();
    foreach (QPrinterInfo printerInfo, m_printerList)
        ui->printerCombo->addItem(printerInfo.description());
    ui->printerCombo->blockSignals(false);

    //  printer
    m_printer = printer;
    onNewPrinter();

    //  MuPDF document (for rendering preview)
    m_document = new Document();
    m_document->Initialize();
    m_document->OpenFile(m_path.toStdString().c_str());

    //  set up slider based on the number of pages
    m_currentPage = currentPage;
    m_maxPages = maxPages;
    setupSlider();

    //  portait by default
    ui->portraitRadio->setChecked(true);

    on_paperSizeComboBox_currentIndexChanged(0);
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

void PrintDialog::on_cancelButton_clicked()
{
    //  user clicked cancel

    onClose();

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

    //  set selected printer
    m_printer->setPrinterName(m_printerList.at(ui->printerCombo->currentIndex()).printerName());

    onClose();

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
    UNUSED(index);

    //  user changed selected printer
    onNewPrinter();
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
    m_timer->start(100);
}

void PrintDialog::onPreviewTimer()
{
    m_timer->stop();
    renderPreview();
}

void PrintDialog::renderPreview()
{
    //  clear the preview
    ui->previewLabel->clear();

    //  get current page number (zero-based)
    QString range = printRange();
    QList<int> pageList = Printer::listFromRange(range, m_maxPages);
    if (pageList.isEmpty())
        return;  //  no pages
    int pageNumber = pageList.at(ui->pageSlider->value()-1)-1;

    //  get the document page dimensions (inches) @ 72DPI
    point_t pageSize;
    m_document->GetPageSize(pageNumber, 1.0, &pageSize);
    double pageWidth = pageSize.X/72.0;
    double pageHeight = pageSize.Y/72.0;

    //  get current paper size
    double paperWidth = m_paperWidth;
    double paperHeight = m_paperHeight;

    //  rotate the paper?
    if (!m_portrait)
    {
        double temp = paperHeight;
        paperHeight = paperWidth;
        paperWidth = temp;
    }

    //  size and place the preview widget within the frame
    int frameh = ui->frame->height();
    int framew = ui->frame->width();
    double scale1 = double(framew)/(paperWidth*72.0);
    double scale2 = double(frameh)/(paperHeight*72.0);
    double scale3 = double(frameh)/(paperWidth*72.0);
    double scale4 = double(framew)/(paperHeight*72.0);
    double scale = 1.0;
    if (scale1<scale) scale = scale1;
    if (scale2<scale) scale = scale2;
    if (scale3<scale) scale = scale3;
    if (scale4<scale) scale = scale4;
    int pw = paperWidth*72.0*scale;
    int ph = paperHeight*72.0*scale;
    ui->previewLabel->setGeometry(framew/2-pw/2,frameh/2-ph/2,pw,ph);

    //  fill widget with white
    ui->previewLabel->setStyleSheet("QLabel { background-color : white; color : white; }");

    //  delete previous page image data
    if (m_image!=NULL)  delete m_image;  m_image=NULL;
    if (m_bitmap!=NULL) delete m_bitmap; m_bitmap=NULL;

    //  render the page
    int numBytes = (pw * ph * 4);
    m_bitmap = new Byte[numBytes];
    m_document->RenderPage (pageNumber, scale, m_bitmap, pw, ph, false);

    //  make an image
    m_image = new QImage(m_bitmap, pw, ph, QImage::Format_ARGB32);

    bool rotate = false;//true;  //  TODO
    if (rotate)
    {
        QTransform tf;
        tf.rotate(90);
        *m_image = m_image->transformed(tf);
    }

    //  set it in the widget
    m_pixmap = QPixmap::fromImage(*m_image);
    ui->previewLabel->setPixmap(m_pixmap);
    ui->previewLabel->update();
}

void PrintDialog::onClose()
{
    //  stop the timer
    m_timer->stop();

    //  delete previous image data
    if (m_image!=NULL)  delete m_image;  m_image=NULL;
    if (m_bitmap!=NULL) delete m_bitmap; m_bitmap=NULL;

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
}

void PrintDialog::onNewPrinter()
{
    //  info for the currently selected printer.
    QPrinterInfo printerInfo = m_printerList.at(ui->printerCombo->currentIndex());

    //  get the paper sizes
    ui->paperSizeComboBox->blockSignals(true);
    ui->paperSizeComboBox->clear();
    m_paperSizes = printerInfo.supportedSizesWithNames();
    for (int i=0; i<m_paperSizes.size() ;i++)
    {
        QPair<QString,QSizeF> paperSize = m_paperSizes.at(i);
        QString name = paperSize.first;
//        QSizeF size = paperSize.second;
        ui->paperSizeComboBox->addItem(name);
    }
    ui->paperSizeComboBox->blockSignals(false);

    //  set the printer name
    m_printer->setPrinterName(printerInfo.printerName());
}

void PrintDialog::on_pageSlider_valueChanged(int value)
{
    setSliderLabel(value);
    updatePreview();
}

void PrintDialog::on_paperSizeComboBox_currentIndexChanged(int index)
{
    //  get selected size
    QPair<QString,QSizeF> paperSize = m_paperSizes.at(index);
    QString name = paperSize.first;
    QSizeF size = paperSize.second;

    //  convert to inches
    double w = size.width();
    double h = size.height();
    m_paperWidth  = w/25.4;
    m_paperHeight = h/25.4;

    //  re-draw the preview
    updatePreview();
}

void PrintDialog::on_portraitRadio_clicked()
{
    m_portrait = true;
    updatePreview();
}

void PrintDialog::on_landscapeRadio_clicked()
{
    m_portrait = false;
    updatePreview();
}
