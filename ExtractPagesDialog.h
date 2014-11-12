#ifndef EXTRACTPAGESDIALOG_H
#define EXTRACTPAGESDIALOG_H

#include <QDialog>

class Window;

namespace Ui {
class ExtractPagesDialog;
}

class ExtractPagesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtractPagesDialog(QWidget *parent = 0);
    ~ExtractPagesDialog();

    void run (Window *win);

private slots:
    void on_cancelButton_clicked();

    void on_extractButton_clicked();

    void on_allButton_clicked();

    void on_evenButton_clicked();

    void on_oddButton_clicked();

    void on_noneButton_clicked();

private:
    Ui::ExtractPagesDialog *ui;
    Window *m_window=NULL;
};

#endif // EXTRACTPAGESDIALOG_H
