#ifndef PROOFSETTINGSDIALOG_H
#define PROOFSETTINGSDIALOG_H

#include <QDialog>

class QFileDialog;

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
    QString getDisplayProfile();

private slots:
    void on_choosePrintProfileButton_clicked();
    void on_chooseSoftProfileButton_clicked();

private:
    Ui::ProofSettingsDialog *ui;

    static QList<QString> m_softProfiles;
    static QList<QString> m_printProfiles;

    QFileDialog *makeICCDialog(QString title);
};

#endif // PROOFSETTINGSDIALOG_H
