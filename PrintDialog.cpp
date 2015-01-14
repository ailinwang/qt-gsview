#include "PrintDialog.h"
#include "ui_PrintDialog.h"

#include <QtPrintSupport/QPrinterInfo>
#include <QDebug>
#include <QPageSetupDialog>


PrintDialog::PrintDialog(QWidget *parent, int numPages, int currentPage) :
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
    m_numPages = numPages;
    ui->pageSlider->setRange(1, m_numPages);
    ui->pageSlider->setValue(m_currentPage);
    setSliderLabel(m_currentPage);

    //  fill in printer names
    m_printerList=QPrinterInfo::availablePrinters();
    foreach (QPrinterInfo printerInfo, m_printerList)
    {
        ui->printerCombo->addItem(printerInfo.description());
    }
}

PrintDialog::~PrintDialog()
{
    delete ui;
}

void PrintDialog::on_propertiesButton_clicked()
{
    //  user clicked properties button

    //  get info for the currently selected printer
    int index = ui->printerCombo->currentIndex();
    QPrinterInfo printerInfo = m_printerList.at(index);

    //qDebug() <<  QString("%1").arg(printerInfo.printerName());

    QPrinter prt(printerInfo);
    QPageSetupDialog dlg(&prt);

    if (dlg.exec() == QDialog::Accepted)
    {

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

    QApplication::restoreOverrideCursor();
    qApp->processEvents();

    QDialog::accept();
}

void PrintDialog::on_allRadioButton_clicked()
{
    //  user clicked all pages radio
}

void PrintDialog::on_currentRadioButton_clicked()
{
    //  user clicked current page radio
}

void PrintDialog::on_pagesRadioButton_clicked()
{
    //  user clicked pages radio
}

void PrintDialog::on_pageListEdit_textChanged()
{
    //  user is typing page numbers, so select the radio button
    ui->pagesRadioButton->setChecked(true);
}

void PrintDialog::on_printerCombo_currentIndexChanged(int index)
{
    //  user changed selected printer
}

void PrintDialog::setSliderLabel(int val)
{
    ui->pageSliderLabel->setText(QString::number(val) + "/" + QString::number(m_numPages));
}

void PrintDialog::on_pageSlider_valueChanged(int value)
{
    setSliderLabel(value);
}
