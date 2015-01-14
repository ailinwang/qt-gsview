#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QDialog>
#include <QtPrintSupport/QPrinterInfo>

namespace Ui {
class PrintDialog;
}

class PrintDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrintDialog(QWidget *parent = 0, int numPages=1, int currentPage=1);
    ~PrintDialog();

private slots:
    void on_cancelButton_clicked();
    void on_printButton_clicked();

    void on_propertiesButton_clicked();
    void on_printerCombo_currentIndexChanged(int index);

    void on_allRadioButton_clicked();
    void on_currentRadioButton_clicked();
    void on_pagesRadioButton_clicked();
    void on_pageListEdit_textChanged();

    void on_pageSlider_valueChanged(int value);

private:
    void setSliderLabel(int val);

    Ui::PrintDialog *ui;
    int m_numPages = 1;
    int m_currentPage = 1;
    QList<QPrinterInfo> m_printerList;

};

#endif // PRINTDIALOG_H
