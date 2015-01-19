#include "PrintDialog.h"
#include "ui_PrintDialog.h"
#include "Printer.h"

#include <QtPrintSupport/QPrinterInfo>
#include <QDebug>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QMessageBox>


PrintDialog::PrintDialog(QWidget *parent, int maxPages, int currentPage, QPrinter *printer) :
    QDialog(parent),
    ui(new Ui::PrintDialog)
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

    //  set up slider based on the number of pages
    m_currentPage = currentPage;
    m_maxPages = maxPages;
    setupSlider();

    //  fill in printer names
    m_printerList=QPrinterInfo::availablePrinters();
    foreach (QPrinterInfo printerInfo, m_printerList)
        ui->printerCombo->addItem(printerInfo.description());

    //  printer
    m_printer = printer;

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

    //  set selected printer
    m_printer->setPrinterName(m_printerList.at(ui->printerCombo->currentIndex()).printerName());

    QApplication::restoreOverrideCursor();
    qApp->processEvents();

    QDialog::accept();
}

void PrintDialog::on_allRadioButton_clicked()
{
    //  user clicked all pages radio
    setupSlider();
}

void PrintDialog::on_currentRadioButton_clicked()
{
    //  user clicked current page radio
    setupSlider();
}

void PrintDialog::on_pagesRadioButton_clicked()
{
    //  user clicked pages radio
    setupSlider();
}

void PrintDialog::on_pageListEdit_textChanged()
{
    //  user is typing page numbers, so select the radio button
    ui->pagesRadioButton->setChecked(true);
    setupSlider();
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

void PrintDialog::on_pageSlider_valueChanged(int value)
{
    setSliderLabel(value);
}
