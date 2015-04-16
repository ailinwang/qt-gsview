#include "PrintDialog.h"
#include "ui_PrintDialog.h"
#include "Printer.h"

#include <QtPrintSupport/QPrinterInfo>
#include <QDebug>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QMessageBox>
#include <QTimer>
#include <QPainter>
#include <QTextStream>

PrintDialog::PrintDialog(QWidget *parent, int maxPages, int currentPage, QPrinter *printer, QString path) :
    QDialog(parent),
    ui(new Ui::PrintDialog),
    m_path(path)
{
    QTextStream(stdout) << "\n" << "PrintDialog ctor 1";

    //  set up the UI
    ui->setupUi(this);
    QTextStream(stdout) << "\n" << "PrintDialog ctor 2";

    //  set the window title
    setWindowTitle(tr("Print Document"));
    QTextStream(stdout) << "\n" << "PrintDialog ctor 3";

    //  insure we've got the arrow cursor.
    qApp->setOverrideCursor(Qt::ArrowCursor);
    qApp->processEvents();
    QTextStream(stdout) << "\n" << "PrintDialog ctor 4";

    //  start with all pages
    ui->allRadioButton->setChecked(true);
    QTextStream(stdout) << "\n" << "PrintDialog ctor 5";

    //  fill in printer names
    //  pre-select the current default printer
    QPrinterInfo defaultInfo = QPrinterInfo::defaultPrinter();
    QTextStream(stdout) << "\n" << "PrintDialog ctor 6";
    ui->printerCombo->blockSignals(true);
    QTextStream(stdout) << "\n" << "PrintDialog ctor 7";
    m_printerList=QPrinterInfo::availablePrinters();
    int ndx = 0;
    foreach (QPrinterInfo printerInfo, m_printerList)
    {
        QTextStream(stdout) << "\n" << "PrintDialog ctor 8, ndx=" << ndx;
        ui->printerCombo->addItem(printerInfo.description());
        if (printerInfo.description() == defaultInfo.description())
            ui->printerCombo->setCurrentIndex(ndx);
        QTextStream(stdout) << "\n" << "PrintDialog ctor 9";
        ndx++;
    }
    ui->printerCombo->blockSignals(false);
    QTextStream(stdout) << "\n" << "PrintDialog ctor 10";

    //  printer
    m_printer = printer;
    onNewPrinter();
    QTextStream(stdout) << "\n" << "PrintDialog ctor 11";

    //  MuPDF document (for rendering preview)
    m_document = new Document();
    m_document->Initialize();
    m_document->OpenFile(m_path.toStdString().c_str());
    QTextStream(stdout) << "\n" << "PrintDialog ctor 12";

    //  set up slider based on the number of pages
    m_currentPage = currentPage;
    m_maxPages = maxPages;
    setupSlider();
    QTextStream(stdout) << "\n" << "PrintDialog ctor 13";

    //  portait by default
    ui->portraitRadio->setChecked(true);

    on_paperSizeComboBox_currentIndexChanged(0);
    QTextStream(stdout) << "\n" << "PrintDialog ctor 14";
}

int PrintDialog::countPrinters()
{
    QList<QPrinterInfo> printerList = QPrinterInfo::availablePrinters();
    return printerList.count();
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

QPair<QString, QSizeF> PrintDialog::paperSize()
{
    return m_paperSizes.at(ui->paperSizeComboBox->currentIndex());
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
    QTextStream(stdout) << "\n" << "setupSlider 1";
    QString range = this->printRange();
    QList<int> pageList = Printer::listFromRange(range, m_maxPages);
    int np = pageList.size();

    QTextStream(stdout) << "\n" << "setupSlider 2";

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

    QTextStream(stdout) << "\n" << "setupSlider 3";

    setSliderLabel(1);

    QTextStream(stdout) << "\n" << "setupSlider 4";
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
    QTextStream(stdout) << "\n" << "renderPreview 1";

    //  clear the preview
    ui->previewLabel->clear();

    //  get current page number (zero-based)
    QString range = printRange();
    QList<int> pageList = Printer::listFromRange(range, m_maxPages);
    if (pageList.isEmpty())
        return;  //  no pages
    int pageNumber = pageList.at(ui->pageSlider->value()-1)-1;
    QTextStream(stdout) << "\n" << "renderPreview 2";

    //  get the document page dimensions (inches) @ 72DPI
    point_t pageSize;
    m_document->GetPageSize(pageNumber, 1.0, &pageSize);
    double pageWidth = pageSize.X/72.0;
    double pageHeight = pageSize.Y/72.0;
    QTextStream(stdout) << "\n" << "renderPreview 3";

    //  get current paper size (inches)
    double paperWidth = m_paperWidth;
    double paperHeight = m_paperHeight;

    //  rotate the "paper" if landscape
    if (!m_portrait)
        {double temp = paperWidth;paperWidth = paperHeight;paperHeight = temp;}

    //  size and place the preview widget within the frame
    int frameh = ui->frame->height();
    int framew = ui->frame->width();

    double scale = 1.0;
    scale = fmin(scale,double(framew)/(paperWidth*72.0));
    scale = fmin(scale,double(frameh)/(paperHeight*72.0));
    scale = fmin(scale,double(frameh)/(paperWidth*72.0));
    scale = fmin(scale,double(framew)/(paperHeight*72.0));

    int pw = paperWidth*72.0*scale;
    int ph = paperHeight*72.0*scale;
    int px = framew/2-pw/2;
    int py = frameh/2-ph/2;

    int offset = 15;
    py +=15;
    ph -=15;

    QTextStream(stdout) << "\n" << "renderPreview 4";
    ui->previewLabel->setGeometry(px,py,pw,ph);

    //  fill widget with white
    ui->previewLabel->setStyleSheet("QLabel { background-color : white; color : white; }");
    QTextStream(stdout) << "\n" << "renderPreview 5";

    //  autofit
    ui->previewLabel->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_autoFitScale = 1.0;
    m_bAutoFitRotate = false;
    if (m_bAutoFit)
    {
        double pageAspect = pageHeight/pageWidth;
        double paperAspect = paperHeight/paperWidth;

        if (pageAspect>1&&paperAspect<1)
        {
            m_bAutoFitRotate = true;
            m_autoFitScale = paperWidth/pageHeight;
            if (pageAspect>1)
                ui->previewLabel->setAlignment(Qt::AlignVCenter);
            else
                ui->previewLabel->setAlignment(Qt::AlignHCenter);

        }
        else if (pageAspect<1&&paperAspect>1)
        {
            m_bAutoFitRotate = true;
            m_autoFitScale = paperHeight/pageWidth;
            if (pageAspect>1)
                ui->previewLabel->setAlignment(Qt::AlignVCenter);
            else
                ui->previewLabel->setAlignment(Qt::AlignHCenter);
        }
        else
        {
            m_autoFitScale = fmin(paperWidth/pageWidth,paperHeight/pageHeight);
            if (pageAspect<1)
                ui->previewLabel->setAlignment(Qt::AlignVCenter);
            else
                ui->previewLabel->setAlignment(Qt::AlignHCenter);
        }
    }
    scale = scale * m_autoFitScale;

    QTextStream(stdout) << "\n" << "renderPreview 6";

    //  render the page to an image
    int w = scale*pageSize.X;
    int h = scale*pageSize.Y;

    int numBytes = (w * h* 4);
    if (m_image!=NULL)  delete m_image;  m_image=NULL;
    if (m_bitmap!=NULL) delete m_bitmap; m_bitmap=NULL;
    m_bitmap = new Byte[numBytes];
    m_document->RenderPage (pageNumber, scale, m_bitmap, w, h, false);
    m_image = new QImage(m_bitmap, w, h, QImage::Format_ARGB32);

    QTextStream(stdout) << "\n" << "renderPreview 7";

    //  rotate
    if (m_bAutoFitRotate)
    {
        QTransform tf;
        tf.rotate(90);
        *m_image = m_image->transformed(tf);
    }

    QTextStream(stdout) << "\n" << "renderPreview 8";

    //  set it in the widget
    m_pixmap = QPixmap::fromImage(*m_image);
    ui->previewLabel->setPixmap(m_pixmap);
    ui->previewLabel->update();

    QTextStream(stdout) << "\n" << "renderPreview 9";

    //  set the geomatry for the size labels.
    //  these get drawn in updateSizeDisplay()
    ui->widthLabel->setGeometry(px, py-offset-2, pw, offset);
    ui->heightLabel->setGeometry(px+pw+3, py, offset, ph);

    //  update size display
    updateSizeDisplay();

    QTextStream(stdout) << "\n" << "renderPreview 10";

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
    QTextStream(stdout) << "\n" << "onNewPrinter 1";

    //  info for the currently selected printer.
    QPrinterInfo printerInfo = m_printerList.at(ui->printerCombo->currentIndex());
    QTextStream(stdout) << "\n" << "onNewPrinter 2";

    //  remember the current page size name
    //  this is a bit of a hack since we really should get the current default
    //  from the printer itself.
    QLocale locale;
    QString countrys = QLocale::countryToString(locale.country());
    QString currentSizeName = "";
    if (countrys == "UnitedStates")
#ifdef _QT_MAC
        currentSizeName = "US Letter";
#else
        currentSizeName = "Letter";
#endif
    else
        currentSizeName = "A4";
    QTextStream(stdout) << "\n" << "onNewPrinter 3";

    if (m_paperSizes.size()>0 && ui->paperSizeComboBox->currentIndex()<m_paperSizes.size())
    {
        QPair<QString,QSizeF> paperSize = m_paperSizes.at(ui->paperSizeComboBox->currentIndex());
        currentSizeName = paperSize.first;
    }
    QTextStream(stdout) << "\n" << "onNewPrinter 4";

    //  get the new paper sizes
    ui->paperSizeComboBox->blockSignals(true);
    ui->paperSizeComboBox->clear();
    m_paperSizes = printerInfo.supportedSizesWithNames();
    int matching = -1;
    for (int i=0; i<m_paperSizes.size() ;i++)
    {
        QTextStream(stdout) << "\n" << "onNewPrinter 5, i = " <<  i;

        QPair<QString,QSizeF> paperSize = m_paperSizes.at(i);
        ui->paperSizeComboBox->addItem(paperSize.first);

        //  see if this is the one we remembered above
        if (currentSizeName.compare(paperSize.first)==0)
            matching = i;
    }
    //  restore the remembered size.
    if (matching!=-1)
        ui->paperSizeComboBox->setCurrentIndex(matching);

    QTextStream(stdout) << "\n" << "onNewPrinter 6";

    on_paperSizeComboBox_currentIndexChanged(ui->paperSizeComboBox->currentIndex());

    ui->paperSizeComboBox->blockSignals(false);

    //  set the printer name
    m_printer->setPrinterName(printerInfo.printerName());
    QTextStream(stdout) << "\n" << "onNewPrinter 7";
}

QString formatDimension(double val)
{
    QString str;
    if (fabs(val - round(val)) < 0.000001)
        str.sprintf("%d", (int)val);
    else
        str.sprintf("%.1f", val);
    return str;
}

void drawText(QPainter & painter, const QPointF & point, int flags,
              const QString & text, QRectF * boundingRect = 0)
{
   const qreal size = 32767.0;
   QPointF corner(point.x(), point.y() - size);
   if (flags & Qt::AlignHCenter) corner.rx() -= size/2.0;
   else if (flags & Qt::AlignRight) corner.rx() -= size;
   if (flags & Qt::AlignVCenter) corner.ry() += size/2.0;
   else if (flags & Qt::AlignTop) corner.ry() += size;
   else flags |= Qt::AlignBottom;
   QRectF rect(corner, QSizeF(size, size));
   painter.drawText(rect, flags, text, boundingRect);
}

void drawText(QPainter & painter, qreal x, qreal y, int flags,
              const QString & text, QRectF * boundingRect = 0)
{
   drawText(painter, QPointF(x,y), flags, text, boundingRect);
}

void drawSizeLabel(QLabel *label, double value, bool rotate)
{
    QTextStream(stdout) << "\n" << "drawSizeLabel 1";

    label->clear();

    int w = label->width();
    int h = label->height();
    if (rotate) {int x=w;w=h;h=x;}

    QPixmap pixmap(w, h);
    pixmap.fill(QColor("transparent"));
    QPainter painter(&pixmap);
    painter.setRenderHint( QPainter::Antialiasing );
    painter.setPen(QPen(Qt::black,1));

    int l = (w-40)/2;
    painter.drawLine(0,h/2,7,h/2-3);
    painter.drawLine(0,h/2,7,h/2+3);
    painter.drawLine(0,h/2,l,h/2);
    painter.drawLine(w,h/2,w-7,h/2-3);
    painter.drawLine(w,h/2,w-7,h/2+3);
    painter.drawLine(w,h/2,w-l,h/2);

    QPointF pt(w/2,h/2);
    painter.setFont(QFont("Helvetica", 10));
    drawText(painter, pt, Qt::AlignVCenter | Qt::AlignHCenter, formatDimension(value));

    painter.end();  // we're done with this painter.

    if (rotate)
    {
        QTransform tf; tf.rotate(90);
        pixmap = pixmap.transformed(tf, Qt::SmoothTransformation);
    }

    label->setPixmap( pixmap );

    QTextStream(stdout) << "\n" << "drawSizeLabel 2";
}

void PrintDialog::updateSizeDisplay()
{
    QTextStream(stdout) << "\n" << "updateSizeDisplay 1";

    //  get current paper size (inches)
    double paperWidth = m_paperWidth;
    double paperHeight = m_paperHeight;
    //  rotate the "paper" if landscape
    if (!m_portrait)
        {double temp = paperWidth;paperWidth = paperHeight;paperHeight = temp;}

    //  convert to cm
    if (ui->cmRadioButton->isChecked())
    {
        paperWidth = 2.54 * paperWidth;
        paperHeight = 2.54 * paperHeight;
    }

    //  draw pixmaps and set them into the labels
    //  for the width and height

    drawSizeLabel(ui->widthLabel,  paperWidth, false);
    drawSizeLabel(ui->heightLabel, paperHeight, true);

    QTextStream(stdout) << "\n" << "updateSizeDisplay 2";

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

void PrintDialog::on_autoFitCheckBox_clicked()
{
    m_bAutoFit = ui->autoFitCheckBox->isChecked();
    updatePreview();
}

void PrintDialog::on_inchesRadioButton_clicked()
{
    updatePreview();
}

void PrintDialog::on_cmRadioButton_clicked()
{
   updatePreview();
}
