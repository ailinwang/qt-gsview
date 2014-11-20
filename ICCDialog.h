#ifndef ICCDIALOG_H
#define ICCDIALOG_H

#include <QDialog>

namespace Ui {
class ICCDialog;
}

class ICCDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ICCDialog(QWidget *parent = 0);
    ~ICCDialog();

    void setPath(QString path) {m_icc_path=path;}
    QString getPath() {return m_icc_path;}

    virtual void show();

private slots:
    void on_selectButton_clicked();

private:
    Ui::ICCDialog *ui;
    QString m_icc_path;
};

#endif // ICCDIALOG_H
