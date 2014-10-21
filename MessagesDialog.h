#ifndef MESSAGESDIALOG_H
#define MESSAGESDIALOG_H

#include <QDialog>

namespace Ui {
class MessagesDialog;
}

class MessagesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessagesDialog(QWidget *parent = 0);
    ~MessagesDialog();

    static void addMessage(QString message);

public slots:
    void onClear();
    void onOK();

protected:
    virtual void showEvent ( QShowEvent * event );

private:
    Ui::MessagesDialog *ui;
    static QString m_messages;
};

#endif // MESSAGESDIALOG_H
