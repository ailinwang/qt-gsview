#ifndef ICCDIALOG2_H
#define ICCDIALOG2_H

#include <QDialog>

namespace Ui {
class ICCDialog2;
}

class ICCDialog2 : public QDialog
{
    Q_OBJECT

public:
    explicit ICCDialog2(QWidget *parent = 0);
    ~ICCDialog2();

    virtual void show();

    static QString rgbProfile()  {return m_rgbProfile;}
    static QString cmykProfile() {return m_cmykProfile;}
    static QString grayProfile() {return m_grayProfile;}

private slots:
    void on_grayButton_clicked();
    void on_rgbButton_clicked();
    void on_cmykButton_clicked();
    void on_okButton_clicked();

private:

    void refreshUI();
    QString askForProfile();


    Ui::ICCDialog2 *ui;

    static QString m_rgbProfile;
    static QString m_cmykProfile;
    static QString m_grayProfile;
};

#endif // ICCDIALOG2_H
