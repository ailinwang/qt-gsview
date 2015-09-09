#ifndef COLORSDIALOG_H
#define COLORSDIALOG_H

#include <QDialog>
class Window;
class Document;
class QCheckBox;
class QVBoxLayout;

namespace Ui {
class ColorsDialog;
}

class ColorsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ColorsDialog (Window *win, Document *doc, int page, QWidget *parent = 0);
    ~ColorsDialog();

    virtual void show();

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::ColorsDialog *ui;
    Window *m_window;
    Document *m_document;
    int m_currentpage;
    int m_numSeps;
    QCheckBox **m_checks = NULL;
};

#endif // COLORSDIALOG_H
