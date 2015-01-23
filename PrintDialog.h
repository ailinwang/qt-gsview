#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QDialog>
#include <QtPrintSupport/QPrinterInfo>

#include "Document.h"

namespace Ui {
class PrintDialog;
}

class PrintDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrintDialog(QWidget *parent = 0, int numPages=1, int currentPage=1, QPrinter *printer=0, QString path="");
    ~PrintDialog();

    QString printRange();
    int copies();

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

    void onPreviewTimer();

private:
    void setSliderLabel(int val);
    void setupSlider();
    void updatePreview();
    void renderPreview();
    void onClose();
    void onNewPrinter();

    Ui::PrintDialog *ui;
    int m_maxPages = 1;
    int m_currentPage = 1;
    QList<QPrinterInfo> m_printerList;
    QPrinter *m_printer;
    QString m_path;
    Document *m_document=0;

    Byte *m_bitmap = NULL;
    QImage *m_image = NULL;
    QPixmap m_pixmap;

    QTimer *m_timer = NULL;

    bool m_portrait = true;
    bool m_printerListBuilt = false;

    QList<QPair<QString,QSizeF>> m_paperSizes;
};

#endif // PRINTDIALOG_H
