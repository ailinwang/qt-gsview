#ifndef PROOFSETTINGSDIALOG_H
#define PROOFSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ProofSettingsDialog;
}

class ProofSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProofSettingsDialog(QWidget *parent = 0);
    ~ProofSettingsDialog();
    int getResolution();
    QString getPrintProfile();
    QString getSoftProfile();

private slots:
    void on_choosePrintProfileButton_clicked();
    void on_chooseSoftProfileButton_clicked();

private:
    Ui::ProofSettingsDialog *ui;
};

#endif // PROOFSETTINGSDIALOG_H
